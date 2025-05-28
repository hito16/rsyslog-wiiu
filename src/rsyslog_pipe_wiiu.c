#include "rsyslog_pipe_wiiu.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h> // For 'bool' type (though no longer strictly needed in this file after refactor)
#include <sys/iosupport.h> // devoptab_list, devoptab_t
#include <whb/log.h>
#include <whb/log_console.h>

#include "simple_discovery_protocol.h"
#include "rsyslog_client.h" // Includes SYSLOG_MESSAGE_MAX_LEN and rsyslog_send_tcp

char SYSLOG_IP[18];

static devoptab_t devoptab;

// The sanitize_syslog_message function has been moved to rsyslog_client.c

ssize_t write_msg_to_syslog(struct _reent *r, void *fd, const char *ptr,
                            size_t len)
{
    char buffer[SYSLOG_MESSAGE_MAX_LEN]; 

    snprintf(buffer, SYSLOG_MESSAGE_MAX_LEN, "%.*s", (int)len, ptr);

    WHBLogPrintf("  a. calling write_msg_to_syslog(%s) with message len %zu\n", SYSLOG_IP, strlen(buffer));
    WHBLogConsoleDraw();

    int res = rsyslog_send_tcp(SYSLOG_IP, SYSLOG_MESSAGE_PORT, 14, buffer);
    WHBLogPrintf("  b. rsyslog_send_tcp returned (%d)\n", res);
    WHBLogConsoleDraw();

    return len; 
}

void init_stdout()
{
    devoptab.name = "STDOUT";
    devoptab.structSize = sizeof devoptab;
    devoptab.write_r = &write_msg_to_syslog;

    devoptab_list[STD_OUT] = &devoptab;
    devoptab_list[STD_ERR] = &devoptab;
}

int init_rsyslog_redirect()
{
    char server_ip_buffer[18];
    if (discovery_get_ip_retry(server_ip_buffer) != 0)
    {
        return 1;
    }
    // tbd - figure out WIIU api for pthread_mutex_lock
    strncpy(SYSLOG_IP, server_ip_buffer, sizeof(SYSLOG_IP) - 1);
    SYSLOG_IP[sizeof(SYSLOG_IP) - 1] = '\0';
    // pthread_mutex_unlock
    // Assuming init_rsyslog_client_mutex() is defined elsewhere or will be added.
    init_rsyslog_client_mutex(); // This call will now be linked from rsyslog_client.c
    init_stdout();
    return 0;
}