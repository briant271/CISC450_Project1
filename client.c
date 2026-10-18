/* 
    Requests a file from the TCP server and saves the received data to out.txt. 
    Receives the files as data packets until EOT packet arrives.
*/

#include <signal.h>
#include "sharedProtocols.h"

int main(int argc, char *argv[])
{
    int sock;
    struct sockaddr_in server;
    unsigned char header[HEADER_SIZE];
    unsigned char buffer[MAX_DATA_SIZE];
    char filename[MAX_FILENAME_SIZE + 2];
    size_t length;
    uint16_t count, sequence_number, expected_sequence = 0;
    unsigned int packet_count = 0;
    unsigned long total_bytes = 0;
    FILE *output_file;

    // check if valid number of command arguments passed
    if (argc > 3)
    {
        fprintf(stderr, "Usage: %s [Server IPv4] [Port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    // default server address and ports used unless overriden
    if (argc >= 2)
        SetAddress(&server, argv[1],
                   argc == 3 ? ParsePort(argv[2]) : SERVER_PORT);
    else
        SetAddress(&server, SERVER_IP, SERVER_PORT);

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        DieWithError("signal");

    // 1. CREATE TCP SOCKET
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock < 0)
        DieWithError("socket");

    // 2. ESTABLISH CONNECTION
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
        DieWithError("connect");

    // 3. COMMUNICATE

    // Get filename from stdin
    printf("Enter filename: ");
    if (fgets(filename, sizeof(filename), stdin) == NULL)
    {
        fprintf(stderr, "Can't read filename.\n");
        close(sock);
        return EXIT_FAILURE;
    }

    // removes newline character from filename if exists
    length = strcspn(filename, "\r\n");
    filename[length] = '\0';

    // Sends the filename as packet 0.
    EncodeHeader(header, (uint16_t)length, 0);

    if (SendAll(sock, header, HEADER_SIZE) < 0 ||
        SendAll(sock, filename, length) < 0)
        DieWithError("send filename");

    // opens output file in binary mode to check if written bytes match received bytes
    output_file = fopen("out.txt", "wb");
    if (output_file == NULL)
        DieWithError("fopen");

    // Receive packets until the EOT packet arrives.
    for (;;)
    {
        // 1 if succesful, 0 if connection closed, negative if error
        int result = RecvExact(sock, header, HEADER_SIZE);

        if (result == 0)
        {
            fprintf(stderr, "Connection closed before EOT packet.\n");
            fclose(output_file);
            close(sock);
            return EXIT_FAILURE;
        }
        if (result < 0)
            DieWithError("recv header");

        // converts count and sequence num from network byte order
        DecodeHeader(header, &count, &sequence_number);

        if (count == 0)
        {
            if (sequence_number != (uint16_t)(expected_sequence + 1))
            {
                fprintf(stderr, "Invalid EOT sequence number.\n");
                fclose(output_file);
                close(sock);
                return EXIT_FAILURE;
            }

            printf("End of Transmission Packet with sequence number %u "
                   "received with 0 data bytes\n",
                   (unsigned int)sequence_number);
            break;
        }

        // check for error in count size or sequence number order
        if (count > MAX_DATA_SIZE ||
            sequence_number != (uint16_t)(expected_sequence + 1))
        {
            fprintf(stderr, "Invalid data packet.\n");
            fclose(output_file);
            close(sock);
            return EXIT_FAILURE;
        }

        // Receive entire data payload before processing it.
        result = RecvExact(sock, buffer, count);
        if (result != 1)
        {
            fprintf(stderr, "Connection closed before full packet data.\n");
            fclose(output_file);
            close(sock);
            return EXIT_FAILURE;
        }

        // Write bytes to preserve original file exactly. 
        if (fwrite(buffer, 1, count, output_file) != count)
            DieWithError("fwrite");

        printf("Packet %u received with %u data bytes\n",
               (unsigned int)sequence_number,
               (unsigned int)count);

        // sets counts for final summary
        expected_sequence = sequence_number;
        packet_count++;
        total_bytes += count;
    }

    // 4. CLOSE CONNECTION

    // closes output file, DieWithError if error
    if (fclose(output_file) != 0)
        DieWithError("fclose");

    // print summary
    printf("Number of data packets received: %u\n", packet_count);
    printf("Total number of data bytes received: %lu\n", total_bytes);

    close(sock);
    return EXIT_SUCCESS;
}
