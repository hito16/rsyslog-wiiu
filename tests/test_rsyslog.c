#include <stdio.h>

#include "rsyslog_client.h"
#include "simple_discovery_protocol.h"

void test_hardcoded_ip()
{
    const char *server_address = "127.0.0.1";
    printf("= Hardcoded IP test\n");
    printf("Sending msg to hardcoded IP address %s\n", server_address);
    // priority 6 = debug
    if (rsyslog_send_tcp(server_address, SYSLOG_MESSAGE_PORT, 6, "1 This is a test message from my C program using TCP!") == 0)
    {
        printf("Syslog message 1 sent successfully to %s\n", server_address);
    }
    else
    {
        perror("Failed to send syslog message.");
    }
}

void test_message_priority()
{
    // Example with a priority 7 and message

    const char *server_address = "127.0.0.1";
    if (rsyslog_send_tcp(server_address, SYSLOG_MESSAGE_PORT, 7, "2 An error occurred in the application!") == 0)
    {
        printf("Syslog message 2 sent successfully to %s\n", server_address);
    }
    else
    {
        perror("Failed to send syslog message.");
    }
    printf("\n");
}

void test_discover_ip()
{
    printf("= Discover IP test\n");
    char server_ip_buffer[17];
    int res = discovery_get_ip_retry(server_ip_buffer);
    if (res == DISCOVERY_SUCCESS)
    {
        printf("Discovered Server IP %s\n", server_ip_buffer);
        if (rsyslog_send_tcp(server_ip_buffer, SYSLOG_MESSAGE_PORT, 6, "3 We discovered your IP on our own") == 0)
        {
            printf("Syslog message sent successfully to %s!\n",
                   server_ip_buffer);
        }
        else
        {
            perror("Failed to send syslog message.");
        }
    }
    else
    {
        perror("Failed to find Server IP");
    }
}

int main()
{

    test_hardcoded_ip();
    test_message_priority();
    test_discover_ip();
}
