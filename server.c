#include <stdio.h>
#include <sys/socket.h> // socket()
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define LISTEN_BACKLOG 5
#define BUFF_SIZE 65536 // 64k

// About TCP (from tcp man page):
// TCP guarantees that the data arrives in order and retransmits lost packets.  It generates  and
// checks a per-packet checksum to catch transmission errors.  TCP does not preserve record boundaries.

void send_response(int fd, char *file_content) {
    char header[100] = {0};
    sprintf(header, "HTTP/1.1 200 OK\nContent-Type:text/html\nContent-Length: %ld\n\n", strlen(file_content));

    char response[BUFF_SIZE] = {0};
    strcat(response, header);
    strcat(response, file_content);

    if(send(fd, response, sizeof(response), 0) < 0) {
        perror("send");
        return;
    }
}

int get_file_content(char *buff, char *path) {
    if(strcmp(path, "/") == 0) path = "/index.html";
    FILE *fp;
    char formatted_path[200] = "./public";
    strcat(formatted_path, path);
    //printf("%s\n", formatted_path);
    if((fp = fopen(formatted_path, "r")) == NULL) {
        return -1;
    }
    if(fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return -1;
    }
    long length = ftell(fp);
    if(fseek(fp, 0, SEEK_SET) != 0) {
        fclose(fp);
        return -1;
    }

    fread(buff, 1, length, fp);
    fclose(fp);

    return 0;
}

char *get_current_time() {
    time_t curr_time = time(NULL);
    return strtok(ctime(&curr_time), "\n");
}

int main(int argc, char *argv[]) {

    // Variables
    unsigned short port;                // port server binds to
    struct sockaddr_in client;          // client addr info
    struct sockaddr_in server;          // server addr info
    int tcp_socket;                     // socket
    int ns;                             // socket connected to client
    int namelen;                        // length og client name 
    char server_addr[20];


    // Check arguments. Should be two, the addr and the port number.
    if(argc != 3) {
        fprintf(stderr, "Usage: %s addr port\n", argv[0]);
        exit(1);
    }

    // Pass the first argument to be the port number.
    port = (unsigned short) atoi(argv[2]);
    strcpy(server_addr, argv[1]);

    // Create the socket.
    if((tcp_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(server_addr);

    // Bind the socket to the server address
    // Traditionally, this  operation  is  called “assigning a name to a socket”. bind(2)
    if(bind(tcp_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        exit(1);
    }

    // Listen for connections.
    if(listen(tcp_socket, LISTEN_BACKLOG) < 0) {
        perror("listen");
        exit(1);
    }

    printf("[%s] Tiny-C-Web-Server (http://%s:%d) started\n", get_current_time(), inet_ntoa(server.sin_addr), ntohs(server.sin_port));

    while(1) {
        // Accept a connection.
        namelen = sizeof(client);
        if((ns = accept(tcp_socket, (struct sockaddr *)&client, (socklen_t *) &namelen)) < 0) {
            perror("accept");
            continue;
        }

        // Successful connection
        printf("[%s] %s:%d Accepted\n", get_current_time(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        char request[BUFF_SIZE] = {0};
        if(recv(ns, request, sizeof(request), 0) < 0) {
            perror("recv");
        }

        char *first_line = strtok(request, "\n");
        char *tokens[3];
        int i = 0; char *token = strtok(first_line, " ");
        while(token != NULL && i < 3) {
            tokens[i] = token;
            token = strtok(NULL, " ");
            i++;
        }
        char *method = tokens[0];
        char *path = tokens[1];
        
        char file_content[BUFF_SIZE] = {0};
        if(get_file_content(file_content, path) < 0) {
            printf("[%s] %s:%d %s %s - %s\n", get_current_time(), inet_ntoa(client.sin_addr), ntohs(client.sin_port), method, path, strerror(errno));
        } else {
            printf("[%s] %s:%d %s %s\n", get_current_time(), inet_ntoa(client.sin_addr), ntohs(client.sin_port), method, path);
            send_response(ns, file_content);
        }

        printf("[%s] %s:%d Closed\n", get_current_time(), inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        close(ns);
    }

    printf("[%s] Server closing\n", get_current_time());
    close(tcp_socket);
    exit(0);
}
