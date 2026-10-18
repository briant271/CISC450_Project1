# Builds the TCP file-transfer server and client.

CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11

TARGETS = server client

.PHONY: all clean

all: $(TARGETS)

server: server.o sharedProtocols.o
	$(CC) $(CFLAGS) -o $@ $^

client: client.o sharedProtocols.o
	$(CC) $(CFLAGS) -o $@ $^

server.o: server.c sharedProtocols.h
	$(CC) $(CFLAGS) -c $< -o $@

client.o: client.c sharedProtocols.h
	$(CC) $(CFLAGS) -c $< -o $@

sharedProtocols.o: sharedProtocols.c sharedProtocols.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGETS) server.o client.o sharedProtocols.o