
#ifndef RSYSLOG_PIPE_WIIU
#define RSYSLOG_PIPE_WIIU

extern char SYSLOG_IP[18];

void init_rsyslog_client_mutex();
int init_rsyslog_redirect();

#endif // RSYSLOG_PIPE_WIIU
