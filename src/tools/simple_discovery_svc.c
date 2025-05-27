#include <stdio.h>
#include <string.h>

#include "simple_discovery_protocol.h"

int main(int argc, char *argv[])
{
    if (argc >= 2 && strcmp(argv[1], "client_test") == 0)
    {
        char server_ip_buffer[17];
        int res = discovery_get_ip_retry(server_ip_buffer);
        if (res == DISCOVERY_SUCCESS)
        {
            fprintf(stdout, "Server ip: %s\n", server_ip_buffer);
        }
        else
        {
            fprintf(stderr, "Error: unable to contact server\n");
        }
        return res;
    }
    else
    {
        fprintf(stdout, "No 'client_test' argument provided or invalid argument. "
                        "Starting discovery service on port %d\n",
                DISCOVERY_PORT);
        discovery_server_acknowledge(DISCOVERY_PORT);
    }

    return 0;
}
