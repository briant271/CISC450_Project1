/* Sets up the server to accept a TCP client request and transmits the requested file in packets. */

#include "sharedProtocols.h"
#include "signal.h"
#define RCVBUFSIZE 80
#define MAXPENDING 5

void HandleTcpClient(int client)
{
    uint16_t header[2];
    uint16_t count, sequence;
    char filename[81];
    char buffer[80];
    FILE *data;
    unsigned long packetCount = 0;
    unsigned long totalBytes = 0;
    int result;

    /* receive header of file name*/
    result = RecvExact(client, header, sizeof(header));
    if (result != 1) {
        fprintf(stderr, "Error receiving filename header.\n");
        close(client);
        return;
    }

    count = ntohs(header[0]);
    sequence = ntohs(header[1]);

    /* check filename sequence number 0 and between 1-80 bytes. */
    if (count == 0 || count > 80 || sequence != 0) {
        fprintf(stderr, "Invalid filename packet\n");
        close(client);
        return;
    }

    // receive the filename 
    result = RecvExact(client, filename, count);
    if (result != 1) {
        fprintf(stderr, "Error receiving filename\n");
        close(client);
        return;
    }

    filename[count] = '\0';


    // open file to parse 
    data = fopen(filename, "r");
    if (data == NULL) {
        fprintf(stderr, "File not found\n");
        close(client);
        return;
    }

    sequence = 1;

    /* Read file and transmit, new packet at \n */
    for (;;) {
        size_t bytes = 0;
        int currentChar;

        while (bytes < sizeof(buffer)) {
            currentChar = fgetc(data);

            if (currentChar == EOF)
                break;

            buffer[bytes++] = (char)currentChar;

            if (currentChar == '\n')
                break;
        }

        if (bytes == 0) {
            if (ferror(data)) {
                perror("Error reading file");
                fclose(data);
                close(client);
                return;
            }
            break;
        }

        if (sequence > 65534) {
            fprintf(stderr, "File too big\n");
            fclose(data);
            close(client);
            return;
        }

        /* order 4 byte header */
        header[0] = htons((uint16_t)bytes);
        header[1] = htons(sequence);

        if (SendAll(client, header, sizeof(header)) < 0 ||
            SendAll(client, buffer, bytes) < 0) {
          fprintf(stderr, "Error sending file data\n");
            fclose(data);
            close(client);
            return;
        }

        printf("Packet %u transmitted with %zu data bytes\n",
               (unsigned int)sequence, bytes);

        packetCount++;
        totalBytes += bytes;
        sequence++;
    }

    fclose(data);

    /* send the EOT packet */
    header[0] = htons(0);
    header[1] = htons(sequence);

    if (SendAll(client, header, sizeof(header)) < 0) {
        fprintf(stderr, "Error sending EOT packet\n");
        close(client);
        return;
    }

    printf("End of Transmission Packet with sequence number %u "
           "transmitted with 0 data bytes\n", (unsigned int)sequence);

    printf("Number of data packets transmitted: %lu\n", packetCount);
    printf("Total number of data bytes transmitted: %lu\n", totalBytes);

    close(client);
}


int main(int argc, char *argv[]){

   
    int server, reuse = 1;
    struct sockaddr_in address;
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        return EXIT_FAILURE;
    }


    SetAddress(&address, NULL, ParsePort(argv[1]));
    
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) DieWithError("signal");

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server < 0) DieWithError("socket");
    /* Allow prompt restart after a previous connection enters TIME_WAIT. */
    if (setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        DieWithError("setsockopt");

    if (bind(server, (struct sockaddr *)&address, sizeof(address)) < 0) DieWithError("bind");

    if (listen(server, MAXPENDING) < 0) DieWithError("listen");

    printf("TCP server listening on port %s\n", argv[1]);

        struct sockaddr_in peer;
        socklen_t peerLength = sizeof(peer);
        int client = accept(server, (struct sockaddr *)&peer, &peerLength);
        // if (client < 0 && errno == EINTR) continue;
        if (client < 0) DieWithError("accept");
        printf("Handling client %s\n", inet_ntoa(peer.sin_addr));
        HandleTcpClient(client); /* Deliberately serve one connection at a time. */
        close(server);
        return EXIT_SUCCESS;
}