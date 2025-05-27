#ifdef __WIIU__

#include <stdio.h>
#include <string.h>
#include <sys/iosupport.h> // devoptab_list, devoptab_t

#include "simple_discovery_protocol.h"
#include "rsyslog_client.h"
#include "rsyslog_pipe_wiiu.h"

char SYSLOG_IP[18];

static devoptab_t devoptab;

ssize_t write_msg_to_syslog(struct _reent *r, void *fd, const char *ptr,
                            size_t len)
{
    char buffer[SYSLOG_MESSAGE_MAX_LEN];
    snprintf(buffer, SYSLOG_MESSAGE_MAX_LEN, "%*.*s", len, len, ptr);
    rsyslog_send_tcp(SYSLOG_IP, SYSLOG_MESSAGE_PORT, 14, buffer);
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
    init_stdout();
    return 0;
}
#endif // __WIIU__
