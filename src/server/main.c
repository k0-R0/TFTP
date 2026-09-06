// the main function for the server
// the server will get 4 kinds of requests
// 1. file request from client
// 2. get a file to the client
// 3. change the operation mode
// 4. disconnect from client and get ready for some other client
#include "commons/commons.h"
#include "commons/logs.h"
#include "server_utils.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

int main() {
    // set up socket and address of the server
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        ERROR_SOCK_CREATE();
        perror(NULL);
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) == 0) {
        ERROR_INVALID_IP(SERVER_IP);
        perror(NULL);
    }
    // bind the socket to server address
    if (bind(server_sock, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        ERROR_BIND_FAILED();
        perror(NULL);
    }
    // server should never stop
    while (1) {
        // receive the initial packet
        struct sockaddr_in client_addr;
        packet pkt;
        socklen_t recv_len = sizeof(client_addr);
        recvfrom(server_sock, &pkt, sizeof(pkt), 0,
                 (struct sockaddr *)&client_addr, &recv_len);
        switch (pkt.opcode) {
        case CONNECT: {
            packet ack;
            ack.opcode = ACK;
            printf("Sender IP : %s\nSender Port : %hu\nData : %s\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                   pkt.data);
            strcpy(ack.data, "data received successfully");
            sendto(server_sock, &ack, sizeof(packet), 0,
                   (struct sockaddr *)&client_addr, sizeof(client_addr));
            break;
        }
        case GET: {
            packet ack;
            ack.opcode = ACK;
            strcpy(ack.data, "GET command received successfully");
            sendto(server_sock, &ack, sizeof(packet), 0,
                   (struct sockaddr *)&client_addr, sizeof(client_addr));
            break;
        }
        case PUT: {
            packet ack;
            ack.opcode = ACK;
            strcpy(ack.data, "PUT command received successfully");
            printf("Sender IP : %s\nSender Port : %hu\nData : %s\n",
                   inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
                   pkt.data);
            sendto(server_sock, &ack, sizeof(packet), 0,
                   (struct sockaddr *)&client_addr, sizeof(client_addr));
            // get file name
            char file_name[200];
            snprintf(file_name, 200, "server_downloads/%s", pkt.data);
            printf("%s", file_name);
            int file_fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file_fd < 0) {
                perror(NULL);
            }
            // write every byte into that file
            recv_file_data(file_fd, &pkt, server_sock, &client_addr);
            break;
        }
        case MODE: {
            packet ack;
            ack.opcode = ACK;
            strcpy(ack.data, "MODE command received successfully");
            sendto(server_sock, &ack, sizeof(packet), 0,
                   (struct sockaddr *)&client_addr, sizeof(client_addr));
            break;
        }
        case QUIT: {
            packet ack;
            ack.opcode = ACK;
            strcpy(ack.data, "QUIT command received successfully");
            sendto(server_sock, &ack, sizeof(packet), 0,
                   (struct sockaddr *)&client_addr, sizeof(client_addr));
            // close or shut down the connection
            break;
        }
        }
    }
}
