

# building library first

In the devkitppc docker instance or your configured WiiU development environment

```
root@devkitppc# cd /project/rsyslog-wiiu
root@devkitppc# make -f Makefile.wiiu.mk
...
librsyslog_wiiu.a
```

# build the example

```
root@devkitppc# cd /project/rsyslog-wiiu/examples/7-rsyslog-test-app
root@devkitppc# make
linking ... rsyslog-test-app.elf
built ... rsyslog-test-app.rpx
Calculating metadata...
Updating sibling and child entries...
Populating data...
Writing header...
Writing /code/root.rpx...
Writing /meta/meta.ini...
Writing dir_hash_table...
Writing dir_table...
Writing file_hash_table...
Writing file_table...
built ... rsyslog-test-app.wuhb
```

# test

## start syslog container in window 1

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

## wiiload the app in window 2

```
root@devkitppc# export WIILOAD=tcp:192.168.0.100
root@devkitppc# wiiload rsyslog-test-app.wuhb
```


# historical notes

The following are notes I took while learning the HB environment and tools.  Passing on to share 
my experiences and motivation.

# Week 3: Stop!! time to address logging

_Caveat: Don't treat these as "proper" examples.  I'm just showing what I did to get up to speed with the homebrew environment.  Look at these, learn and avoid the same bruises_

### Problem:
* the "right" solution is to buy a USB serial cable.
* UDP is way to lossy to be useful
* SDL is the death of screen logging.  Outputting logs to the screen was annoying enough.  Once we delve into SDL graphics, we can no longer rely on outputting logs to the same screens we're drawing on.

### Stopgap:
TCP logging to rsyslogd server

A few devs suggested I implement tcp logging, and showed code samples.  That
gave me the idea of just doing rsyslog.  So here it is.

1. link in a library that redirects stdout and stderr to tcpip/syslog
2. run a docker config that simply starts rsyslogd in a container,
3. for ease of use, run a simple daemon that responds with the IP of the rsyslogd service

Result:  
Reliable printf() logging that feels like a normal posix app logging to syslog.
Easy setup with Docker, using services built into linux.
Standard, hi perf, configurable logging server, with formatting options and UI integrations.

* thanks to DanielKO and TurtleP for suggestion on TCP logging and pointers to redirecting stdout

## TODO
1. Look at the README.md in ../rsyslog for starting Dockerfile.rsyslog
2. As for this Wiiu app, Build it, run it.
3. docker exec into the Dockerfile.rsyslog and tail /var/log/remote/*

```
make
export WIILOAD=tcp:192.168.0.87
wiiload *.rpx
```

If you don't start the Docker image, the app will hang.

## Things I learned
1. With this last piece, we can now build a generic WiiU main() app
3. We can leave printf() fprintf() in place without code changes and see them in rsyslog.
2. single threaded, posix / SDL code that runs linux or MacOS could run on WiiU largely unaltered

