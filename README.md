
## Remote Syslog debugging

A simple TCP remote logging solution, targetting WIIU homebrew development.

* thanks to DanielKO and TurtleP for suggestion to implement TCP logging and pointers to redirecting stdout

### Starting Syslog Server

```
# Build the docker container
docker build -t rsyslogd-wiiu-9514 -f Dockerfile.rsyslogd  .

# Start the container in the background
docker run -d --name rsyslogd-wiiu-9514 --hostname rsyslogd-wiiu-9514 -p 9514:9514/tcp -p 9514:9514/udp -p 9515:9515/udp -v rsyslogd-wiiu-9514:/var/log/remote rsyslogd-wiiu-9514

# Browse / tail the remote log files 
docker exec -it rsyslogd-wiiu-9514 /bin/bash

tail -f /var/log/remote/*
2025-04-29T00:24:28.461614+00:00 Test Syslog message
```

### Building client library (WiiU)

```
root@devkitppc:/project/rsyslog-wiiu#  make -f Makefile.wiiu.mk
...
librsyslog_wiiu.a
```

### Use in Makefiles (WiiU)

```
RSYSLOG_ROOT := /project/rsyslog-wiiu
CFLAGS += -I$(RSYSLOG_ROOT)/include
LDFLAGS += -L$(RSYSLOG_ROOT)/lib  -lrsyslog_wiiu
```

### More info

See docs/ for code usage

