#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
// #include <sys/time.h> // Removed as SO_SNDTIMEO/SO_RCVTIMEO are undeclared in WiiU headers
#include <stdbool.h> // Required for bool type in sanitize_syslog_message

// Conditional include and mutex definition
#ifdef __WIIU__
#include <sys/iosupport.h>         // For _LOCK_T, __SYSCALL(lock_init), etc.
static _LOCK_T rsyslog_send_mutex; // Wii U OS mutex using _LOCK_T
#else
#include <pthread.h> // For POSIX pthreads
static pthread_mutex_t rsyslog_send_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

// Global state for connection reuse
static int rsyslog_sockfd = -1;
static bool rsyslog_connected = false;

const int SYSLOG_MESSAGE_MAX_LEN = 1024;
const int SYSLOG_MESSAGE_PORT = 9154;

// Forward declarations for functions defined later in this file
void init_rsyslog_client_mutex();
void deinit_rsyslog_client_mutex();
void rsyslog_client_lock();
void rsyslog_client_unlock();

static void sanitize_syslog_message(char *dest_buffer, size_t dest_buffer_size, const char *src_ptr, size_t src_len)
{
    bool ends_with_newline = false;

    if (src_len > 0 && src_ptr[src_len - 1] == '\n')
    {
        ends_with_newline = true;
    }

    size_t max_content_len;
    if (ends_with_newline)
    {
        max_content_len = dest_buffer_size - 1;
    }
    else
    {
        max_content_len = dest_buffer_size - 2;
    }

    size_t chars_to_copy = (src_len < max_content_len) ? src_len : max_content_len;

    snprintf(dest_buffer, chars_to_copy + 1, "%.*s", (int)chars_to_copy, src_ptr);

    if (!ends_with_newline)
    {
        if (chars_to_copy < max_content_len)
        {
            dest_buffer[chars_to_copy] = '\n';
            dest_buffer[chars_to_copy + 1] = '\0';
        }
        else
        {
            // truncate
            if (dest_buffer_size >= 2)
            {
                dest_buffer[dest_buffer_size - 2] = '\n';
                dest_buffer[dest_buffer_size - 1] = '\0';
            }
        }
    }
}

// Function to initialize the mutex for rsyslog_client
void init_rsyslog_client_mutex()
{
#ifdef __WIIU__
    __SYSCALL(lock_init)(&rsyslog_send_mutex);
#else
    // For pthreads, PTHREAD_MUTEX_INITIALIZER handles initialization
#endif
}

// Function to deinitialize the mutex for rsyslog_client
void deinit_rsyslog_client_mutex()
{
#ifdef __WIIU__
    __SYSCALL(lock_close)(&rsyslog_send_mutex);
#else
    pthread_mutex_destroy(&rsyslog_send_mutex);
#endif
}

// Function to acquire the rsyslog_client mutex
void rsyslog_client_lock()
{
#ifdef __WIIU__
    __SYSCALL(lock_acquire)(&rsyslog_send_mutex);
#else
    pthread_mutex_lock(&rsyslog_send_mutex);
#endif
}

// Function to release the rsyslog_client mutex
void rsyslog_client_unlock()
{
#ifdef __WIIU__
    __SYSCALL(lock_release)(&rsyslog_send_mutex);
#else
    pthread_mutex_unlock(&rsyslog_send_mutex);
#endif
}

/* A fairly cookie cutter approach to writing to a remote syslogd over TCP.
   This implementation supports connection reuse and is thread-safe.
   Without SO_SNDTIMEO/SO_RCVTIMEO, network operations may block indefinitely
   if the server is unreachable or unresponsive.
*/
int rsyslog_send_tcp(const char *syslog_server_ip, int port, int priority,
                     const char *message)
{
    // Acquire mutex at the beginning of the function to ensure thread safety
    rsyslog_client_lock();

    int ret_val = 0; // Default return value
    char syslog_msg[SYSLOG_MESSAGE_MAX_LEN];
    time_t now;
    struct tm *tm_info;

    // 1. Connection management: reuse socket if connected, otherwise establish
    if (!rsyslog_connected || rsyslog_sockfd == -1)
    {
        // Close existing socket if any (in case connection was lost)
        if (rsyslog_sockfd != -1)
        {
            close(rsyslog_sockfd);
            rsyslog_sockfd = -1;
        }

        if ((rsyslog_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("rsyslogd socket failed");
            rsyslog_client_unlock(); // Release mutex before returning
            return -1;
        }

        // SO_SNDTIMEO and SO_RCVTIMEO don't seem to exist
        //

        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port);

        if (inet_pton(AF_INET, syslog_server_ip, &servaddr.sin_addr) <= 0)
        {
            perror("rsyslogd inet_pton failed");
            close(rsyslog_sockfd);
            rsyslog_sockfd = -1;
            rsyslog_client_unlock(); // Release mutex before returning
            return -2;
        }

        if (connect(rsyslog_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        {
            perror("rsyslogd connect failed");
            close(rsyslog_sockfd);
            rsyslog_sockfd = -1;
            rsyslog_client_unlock(); // Release mutex before returning
            return -3;
        }

        rsyslog_connected = true;
    }

    char sanitized_message[SYSLOG_MESSAGE_MAX_LEN];
    sanitize_syslog_message(sanitized_message, sizeof(sanitized_message), message, strlen(message));

    // Format syslog message (RFC 3164)
    time(&now);
    tm_info = localtime(&now);
    strftime(syslog_msg, SYSLOG_MESSAGE_MAX_LEN, "<%d> %b %d %H:%M:%S WIIU: ", tm_info);
    strncat(syslog_msg, sanitized_message, SYSLOG_MESSAGE_MAX_LEN - strlen(syslog_msg) - 1);

    if (send(rsyslog_sockfd, syslog_msg, strlen(syslog_msg), 0) < 0)
    {
        perror("rsyslogd send failed; marking connection for reconnect");
        // Sending failed, connection might be broken. Force reconnect on next call.
        rsyslog_connected = false;
        close(rsyslog_sockfd); // Close the bad socket
        rsyslog_sockfd = -1;
        ret_val = -4;
    }

    rsyslog_client_unlock();
    return ret_val;
}