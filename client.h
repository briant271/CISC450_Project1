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
#endif
