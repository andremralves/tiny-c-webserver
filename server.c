#include <stdio.h>

#include <sys/socket.h> // socket()
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <stdlib.h>
#include <unistd.h>

#define LISTEN_BACKLOG 1

// About TCP (from tcp man page):
// TCP guarantees that the data arrives in order and retransmits lost packets.  It generates  and
// checks a per-packet checksum to catch transmission errors.  TCP does not preserve record boundaries.

int main(int argc, char *argv[]) {

    // Variables
    unsigned short port;            // port server binds to
    struct sockaddr_in client;      // client addr info
    struct sockaddr_in server;      // server addr info
    int tcp_socket;                 // socket
    int ns;                         // socket connected to client
    int namelen;                    // length og client name 
    char buff[100];                  // buffer for sending and receiving data

    // Check arguments. Should be only one, the port number.
    if(argc != 2) {
        fprintf(stderr, "Usage: %s port\n", argv[0]);
        exit(1);
    }

    // Pass the first argument to be the port number.
    port = (unsigned short) atoi(argv[1]);


    // Create the socket.
    if((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "socket() error");
        exit(2);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the server address
    // Traditionally, this  operation  is  called “assigning a name to a socket”. bind(2)
    if(bind(tcp_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        fprintf(stderr, "bind() error");
        exit(3);
    }

    // Listen for connections.
    if(listen(tcp_socket, LISTEN_BACKLOG) < 0) {
        fprintf(stderr, "listen() error");
        exit(4);
    }

    // Accept a connection.
    namelen = sizeof(client);
    if((ns = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t *) &namelen)) < 0) {
        fprintf(stderr, "accept() error");
        exit(5);
    }

    // Receive the message on the newly connected socket.
    if(recv(ns, buff, sizeof(buff), 0) < 0) {
        fprintf(stderr, "recv() error");
        exit(6);
    }

    // send the message back to the client
    if(send(ns, buff, sizeof(buff), 0) < 0) {
        fprintf(stderr, "send() error");
        exit(7);
    }

    close(ns);
    close(tcp_socket);

    printf("Server ended with no errors\n");
    exit(0);
}
