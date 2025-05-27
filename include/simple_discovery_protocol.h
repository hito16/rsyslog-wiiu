#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <stddef.h>

extern const int DISCOVERY_PORT;
extern const size_t DISCOVERY_BUFFER_SIZE;
extern const char *const DISCOVERY_BROADCAST_IP;
extern const int DISCOVERY_SUCCESS;

int discovery_client_announce(char *server_ip_buffer, const int server_port);
int discovery_server_acknowledge(const int server_port);
int discovery_get_ip_retry(char *server_ip_buffer);

#endif // DISCOVERY_H