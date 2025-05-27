#ifndef RSYSLOG_H
#define RSYSLOG_H

extern const int SYSLOG_MESSAGE_PORT;

int rsyslog_send_tcp(const char *syslog_server_ip, int port, int priority,
                     const char *message);

#endif  // RSYSLOG_H
