/* Constants and function declarations shared by the client and server. */

#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#define DEFAULT_PORT 5000

void DieWithError(const char *message);
unsigned short ParsePort(const char *text);
void SetAddress(struct sockaddr_in *address, const char *ip, unsigned short port);
int SendAll(int socket, const void *buffer, size_t length);
int RecvExact(int socket, void *buffer, size_t length);
void EncodeHeader(unsigned char *header,
                  uint16_t count,
                  uint16_t sequence_number);
void DecodeHeader(const unsigned char *header,
                  uint16_t *count,
                  uint16_t *sequence_number);

#endif