/* 
    Helper functions shared by the client and server: error handling, address setup, 
    send/receive, and packet header encoding. 
*/
#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <stdint.h>
#include <stddef.h>

#define SERVER_PORT 5000
#define SERVER_IP "127.0.0.1"

#define MAX_DATA_SIZE 80
#define MAX_FILENAME_SIZE 80
#define HEADER_SIZE 4

void DieWithError(const char *message)
{
    perror(message); /* Only use for a failed system/library call with errno set. */
    exit(EXIT_FAILURE);
}

unsigned short ParsePort(const char *text)
{
    char *end;
    long port;
    errno = 0;
    port = strtol(text, &end, 10);
    if (*text == '\0' || *text < '0' || *text > '9' || *end != '\0' ||
        errno != 0 || port < 1 || port > 65535)
    {
        fprintf(stderr, "Port must be an integer from 1 to 65535.\n");
        exit(EXIT_FAILURE);
    }
    return (unsigned short)port;
}

void SetAddress(struct sockaddr_in *address, const char *ip, unsigned short port)
{
    memset(address, 0, sizeof(*address));
    address->sin_family = AF_INET;
    address->sin_port = htons(port); /* Network byte order. */
    if (ip == NULL)
        address->sin_addr.s_addr = htonl(INADDR_ANY);
    else if (inet_pton(AF_INET, ip, &address->sin_addr) != 1)
    {
        fprintf(stderr, "Invalid IPv4 address: %s\n", ip);
        exit(EXIT_FAILURE);
    }
}

// Send all requested bytes, even if send() sends only part of them.
int Send_all(int socket, const void *buffer, size_t length)
{
    const char *ptr = buffer;
    size_t sent = 0;
    while (sent < length)
    {
        ssize_t n = send(socket, ptr + sent, length - sent, 0);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return -1;
        sent += (size_t)n;
    }
    return 0;
}

/* Receive exactly the requested bytes.
   Returns 1 on success, 0 if the connection closes before any bytes,
   and -1 on an error or an incomplete message. */
int Recv_exact(int socket, void *buffer, size_t length)
{
    char *ptr = buffer;
    size_t received = 0;
    while (received < length)
    {
        ssize_t n = recv(socket, ptr + received, length - received, 0);
        if (n < 0 && errno == EINTR)
            continue;
        if (n < 0)
            return -1;
        if (n == 0)
            return received == 0 ? 0 : -1;
        received += (size_t)n;
    }
    return 1;
}

void EncodeHeader(unsigned char *header,
                  uint16_t count,
                  uint16_t sequence_number)
{
    uint16_t network_count = htons(count);
    uint16_t network_sequence = htons(sequence_number);

    memcpy(header, &network_count, sizeof(network_count));
    memcpy(header + sizeof(network_count),
           &network_sequence,
           sizeof(network_sequence));
}

void DecodeHeader(const unsigned char *header,
                  uint16_t *count,
                  uint16_t *sequence_number)
{
    uint16_t network_count;
    uint16_t network_sequence;

    memcpy(&network_count, header, sizeof(network_count));
    memcpy(&network_sequence,
           header + sizeof(network_count),
           sizeof(network_sequence));

    *count = ntohs(network_count);
    *sequence_number = ntohs(network_sequence);
}
