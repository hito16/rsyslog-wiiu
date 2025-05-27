

# WiiU 

### building WiiU

From the project root
```
devkitppc% make -f Makefile.wiiu.mk
librsyslog_wiiu.a
```

This will produce a static library in ./lib

### usage (WiiU)
Instead of doing explict logging yourselff, call this and all stdout/stderr
will be redirected to syslog automatically.

Note, this is intended for debugging, and will block until the discovery service is reachable.

```
    #ifdef DEBUG
    #include "rsyslog_pipe_wiiu.h"
    ...
        if (init_rsyslog_redirect() == 0) {
             ...
        }
    #endif

    // custom logging no longer needed.  anything STDOUT/STDERR is sent to syslog
    printf();
```

# Linux and MacOS

### building Linux and MacOS 

From the project root
```
% make
```

### usage Linux and MacOS

```
   char server_ip_buffer[18];
   if (discovery_get_ip_retry(server_ip_buffer) != 0)
   char *message = "text line to send to syslog server";
    if (rsyslog_send_tcp(server_ip_buffer, port, priority, message) == 0) {
         // 
    }
}
```





### Using Discovery Service  Linux and MacOS

##### discover and send test message

Window 1 start service
```
% ./simple_discovery_svc 
No 'client_test' argument provided or invalid argument. Starting discovery service on port 9515
Server listening on port 9515
```

Window 2 send a client_test message
```
% ./simple_discovery_svc client_test  
Client sending to 255.255.255.255:9515: "Hello, Any Servers? Please send your IP."
Client received from 192.168.0.97:9515: "Hello, Broadcasting Client! My IP is in this packet"
SERVER_IP=192.168.0.97
```

## Running tests Linux and MacOS

```

% ./tests/test_rsyslog    
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
