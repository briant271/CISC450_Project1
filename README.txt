# CISC450_Project1

## 1. List of Relevant Files
    | Filename | Purpose |
    | --- | --- |
    | 'README.md' | Setup, usage, explanations, and troubleshooting |
    | 'client.c' | 
            Connects to the TCP server, requests a file, and writes the received
            data to out.txt. 
        |
    | 'server.c' | 
        Creates a TCP server, receives a filename, and transmits the
        requested file using the required packet format. 
        |
    | 'sharedProtocols.c' | shared functions between server.c and client.c |
    | 'sharedProtocols.h | header file for sharedProtcols, common declartions |
    | `Makefile` | Builds executables|

## 2. Compilation Instructions

    From the `CISC450_Project1` directory, run:

        make

    The Makefile builds these executables ('make clean' to remove):

        server
        client

## 3. Configuration information
    The shared settings are defined in sharedProtocols.h:

    - Default server address: 127.0.0.1
    - Default server port: 5000
    - Maximum filename size: 80 bytes
    - Maximum data packet size: 80 bytes
    - Packet header size: 4 bytes

## 4. Running Instructions
    Place the file to be transferred in the server's current working
    directory. Open two terminals in the `CISC450_Project1` directory.

    In the first terminal, start the server with a port number:

        ./server 5000

    In the second terminal, start the client using the server address and
    port:

        ./client 127.0.0.1 5000

    Enter the filename to request (with .txt included). The server reads that file,
    divides it into packets of up to 80 data bytes, and sends the packets to
    the client. The client saves the received contents as `out.txt`.

    The client can use the default address and port by running:

        ./client

    To provide only a server address and use the default port:

        ./client 192.168.1.20


## 5. Group Information (IDENTIFY SOLE AUTHOR BEFORE SUBMISSION)
    Members:
    - Brian Tran, 702657631
        - Contributions:
            - Implementation of client.c and shareprotocols.h
            - edited Readme
        
    - Daven Kaufmann 702753827 
        - Contributions:
            - Implementation of server.c and sharedprotols.h
            - edited Readme
            - implement Makefile