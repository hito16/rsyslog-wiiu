#ifdef __WIIU__
#ifndef RSYSLOG_WIIU
#define RSYSLOG_WIIU

//#include <stddef.h>

extern char SYSLOG_IP[18];

int init_rsyslogger();

#endif  // RSYSLOG_WIIU
#endif  // __WIIU__
