// the main function for the server
// the server will get 4 kinds of requests
// 1. file request from client
// 2. get a file to the client
// 3. change the operation mode
// 4. disconnect from client and get ready for some other client
#include "commons/commons.h"
#include "commons/logs.h"
#include <arpa/inet.h>
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
    server_addr.sin_port = SERVER_PORT;
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
    // while (1) {
    // receive the initial packet
    struct sockaddr_in client_addr;
    char cmd_buffer[100];
    socklen_t recv_len = sizeof(client_addr);
    recvfrom(server_sock, cmd_buffer, 100, 0, (struct sockaddr *)&client_addr,
             &recv_len);
    printf("Sender IP : %s\nSender Port : %hu\nData : %s",
           inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port),
           cmd_buffer);
    // }
}
