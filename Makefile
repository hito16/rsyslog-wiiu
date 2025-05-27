# Makefile

CC = gcc
CFLAGS = -I./include

# Source file definitions
SRC_DEPS	=	src/rsyslog_client.c src/simple_discovery_protocol.c
SRCS_DISCOVERY_SVC	= 	src/tools/simple_discovery_svc.c $(SRC_DEPS)
SRCS_TEST_RSYSLOG	=	tests/test_rsyslog.c $(SRC_DEPS)

# Output executable definitions
TARGET_TEST_RSYSLOG = tests/test_rsyslog
TARGET_DISCOVERY_SVC = simple_discovery_svc

# Combine targets and sources into lists
TARGETS = $(TARGET_TEST_RSYSLOG) $(TARGET_DISCOVERY_SVC) $(TARGET_ACK_SVC)

all: $(TARGETS)

$(TARGET_TEST_RSYSLOG): $(SRCS_TEST_RSYSLOG)
	$(CC) $(CFLAGS) $(SRCS_TEST_RSYSLOG) -o $(TARGET_TEST_RSYSLOG)

$(TARGET_DISCOVERY_SVC): $(SRCS_DISCOVERY_SVC)
	$(CC) $(CFLAGS) $(SRCS_DISCOVERY_SVC) -o $(TARGET_DISCOVERY_SVC)


clean:
	rm -f $(TARGET_TEST_RSYSLOG) $(TARGET_DISCOVERY_SVC) 

.PHONY: all clean
