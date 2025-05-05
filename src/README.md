
# usage - All Platforms

```
if (find_syslog_ip(server_ip_buffer) == 0) {
   char *message = "text line to send to syslog server";
    if (rsyslog_send_tcp(server_ip_buffer, port, priority, message) == 0) {
         // 
    }
}
```

# usage - WiiU

You don't need to use the explicit logging, instead, do this.

```
    #ifdef DEBUG
        if (init_rsyslogger() == 0) {
             ...
        }
    #endif

    // custom logging no longer needed.  anything STDOUT/STDERR is sent to syslog
    printf();
```


# Testing code samples

## building
```
% make
gcc -I. *.c tools/test_rsyslog.c -o test_rsyslog
gcc -I. *.c tools/udp_announce_cli.c -o udp_announce_cli
gcc -I. *.c tools/udp_acknowledge_svc.c -o udp_acknowledge_svc
```

## discover and send test message

```
% ./udp_announce_cli   
Client sending to 255.255.255.255:9515: "Hello, Any Servers? Please send your IP."
Client received from 192.168.0.97:9515: "Hello, Broadcasting Client! My IP is in this packet"
SERVER_IP=192.168.0.97
```

## sample of both hard coded IP and discovered IP

```
% ./test_rsyslog    
= Hardcoded IP test
Sending msg to hardcoded IP address 127.0.0.1
Syslog message 1 sent successfully to 127.0.0.1
Syslog message 2 sent successfully to 127.0.0.1
```

= Discover IP test
Client sending to 255.255.255.255:9515: "Hello, Any Servers? Please send your IP."
Client received from 192.168.0.97:9515: "Hello, Broadcasting Client! My IP is in this packet"
Discovered Server IP 192.168.0.97
Syslog message sent successfully to 192.168.0.97!
```
