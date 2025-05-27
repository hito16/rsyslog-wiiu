#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

const int SYSLOG_MESSAGE_MAX_LEN = 1024;
const int SYSLOG_MESSAGE_PORT = 9154;
/* A fairly cookie cutter approach to writing to a remote syslogd over
   TCP.
   TBD: provide connection reuse, pooling  (requires thread safety)
*/
int rsyslog_send_tcp(const char *syslog_server_ip, int port, int priority,
                     const char *message)
{
    int sockfd;
    struct sockaddr_in servaddr;
    char syslog_msg[SYSLOG_MESSAGE_MAX_LEN];
    time_t now;
    struct tm *tm_info;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("rsyslogd socket failed");
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (inet_pton(AF_INET, syslog_server_ip, &servaddr.sin_addr) <= 0)
    {
        perror("rsyslogd inet_pton failed");
        close(sockfd);
        return -2;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("rsyslogd connect failed");
        close(sockfd);
        return -3;
    }

    // Format syslog message (RFC 3164)
    time(&now);
    tm_info = localtime(&now);
    strftime(syslog_msg, SYSLOG_MESSAGE_MAX_LEN, "<%d> %b %d %H:%M:%S WIIU: ", tm_info);
    strncat(syslog_msg, message, SYSLOG_MESSAGE_MAX_LEN - strlen(syslog_msg) - 1);

    if (send(sockfd, syslog_msg, strlen(syslog_msg), 0) < 0)
    {
        perror("rsyslogd send failed");
        close(sockfd);
        return 1;
    }

    close(sockfd);
    return 0;
}
