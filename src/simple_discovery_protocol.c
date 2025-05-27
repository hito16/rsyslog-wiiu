#include "simple_discovery_protocol.h"

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Simple Announce protocol globals
const int DISCOVERY_PORT = 9515;
const size_t DISCOVERY_BUFFER_SIZE = 1024;
const char *const DISCOVERY_BROADCAST_IP = "255.255.255.255";

const int DISCOVERY_SUCCESS = 0;

static void error(const char *msg) { perror(msg); }

/* Broadcast a UDP packet to SA_SERVER_PORT, return IP of the first service
   who responds.
*/
int discovery_client_announce(char *server_ip_buffer, const int server_port)
{
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[DISCOVERY_BUFFER_SIZE];
    const char *message = "Hello, Any Servers? Please send your IP.";
    int broadcastEnable = 1;

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket");
        return 1;
    }

    /*  SO_RCVTIMEO does not exist within Devkitpro (ppc) / WUT
        We could do async io in a loop with poll() and select() if needed.
    struct timeval read_timeout;
    read_timeout.tv_sec = 1;
    read_timeout.tv_usec = 0;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &read_timeout,
               sizeof read_timeout);
    */

    // Set socket option for broadcasting
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable,
                   sizeof(broadcastEnable)) < 0)
    {
        error("Error setting socket option for broadcast");
        return 1;
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, DISCOVERY_BROADCAST_IP, &server_addr.sin_addr) <= 0)
    {
        error("inet_pton failed");
        return 1;
    }

    // Send message to the broadcast address
    printf("Client sending to %s:%d: \"%s\"\n", DISCOVERY_BROADCAST_IP, server_port,
           message);
    if (sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        error("Error sending message");
        return 1;
    }

    // Receive response from server
    socklen_t server_len = sizeof(server_addr);
    memset(buffer, 0, DISCOVERY_BUFFER_SIZE);
    if (recvfrom(sockfd, buffer, DISCOVERY_BUFFER_SIZE, 0,
                 (struct sockaddr *)&server_addr, &server_len) < 0)
    {
        error("Error receiving response"); // Changed to error, client expects
                                           // a response.
        return 1;
    }

    // Extract server IP address.  Be cause the server responded, we
    // can pull it's IP address from the response packet
    inet_ntop(AF_INET, &server_addr.sin_addr, server_ip_buffer,
              INET_ADDRSTRLEN);
    printf("Client received from %s:%d: \"%s\"\n", server_ip_buffer,
           ntohs(server_addr.sin_port), buffer);

    close(sockfd);
    return DISCOVERY_SUCCESS;
}

/* The server listens for UDP broadcasts to a specific port, responds */
int discovery_server_acknowledge(const int server_port)
{
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[DISCOVERY_BUFFER_SIZE];

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        error("Error opening socket");
    }

    // Configure server address
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr =
        INADDR_ANY; // Bind to any available interface
    server_addr.sin_port = htons(server_port);

    // Bind socket to address
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
        0)
    {
        error("Error binding socket");
    }

    printf("Server listening on port %d\n", server_port);

    while (1)
    {
        socklen_t client_len = sizeof(client_addr);
        memset(buffer, 0, DISCOVERY_BUFFER_SIZE);

        // Receive message from client
        if (recvfrom(sockfd, buffer, DISCOVERY_BUFFER_SIZE, 0,
                     (struct sockaddr *)&client_addr, &client_len) < 0)
        {
            error("Error receiving message");
        }

        printf("Server received from %s:%d: \"%s\"\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
               buffer);

        // Respond to the client, including the server's IP address
        char server_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &server_addr.sin_addr, server_ip_str,
                  INET_ADDRSTRLEN);
        char response[DISCOVERY_BUFFER_SIZE];
        snprintf(response, DISCOVERY_BUFFER_SIZE,
                 "Hello, Broadcasting Client! My IP is in this packet");

        if (sendto(sockfd, response, strlen(response), 0,
                   (struct sockaddr *)&client_addr, client_len) < 0)
        {
            error("Error sending response");
        }
        printf("Server sent response to %s:%d: \"%s\"\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
               response);
    }

    close(sockfd);
    return 0;
}

int discovery_get_ip_retry(char *server_ip_buffer)
{
    int res = -1;
    for (int i = 0; i < 3; i++)
    {
        res = discovery_client_announce(server_ip_buffer, DISCOVERY_PORT);
        if (res == DISCOVERY_SUCCESS)
        {
            return res;
        }
    }
    return res;
}