// the main function for the server
#include "commons/commons.h"
#include "commons/logs.h"
#include "server_utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int main() {
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        ERROR_SOCK_CREATE();
        perror(NULL);
        return FAILURE;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) == 0) {
        ERROR_INVALID_IP(SERVER_IP);
        perror(NULL);
        close(server_sock);
        return FAILURE;
    }

    if (bind(server_sock, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        ERROR_BIND_FAILED(SERVER_PORT);
        perror("bind");
        close(server_sock);
        return FAILURE;
    }

    TransferMode current_mode = MODE_OCTET;
    char rxBuffer[sizeof(data_packet)];

    printf("TFTP Server listening on %s:%d\n", SERVER_IP, SERVER_PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t recv_len = sizeof(client_addr);
        if (recvfrom(server_sock, rxBuffer, sizeof(rxBuffer), 0,
                     (struct sockaddr *)&client_addr, &recv_len) < 0) {
            ERROR_SERVER_LISTENER();
            continue;
        }

        FileContext ctx = {
            .sock_fd = server_sock,
            .dest_addr = client_addr,
            .pkt = *(cmd_packet *)rxBuffer,
            .mode = current_mode
        };

        switch (rxBuffer[0]) {
        case CONNECT:
            handle_connect(&ctx);
            break;
        case GET:
            handle_get(&ctx);
            break;
        case PUT:
            handle_put(&ctx);
            break;
        case MODE:
            handle_mode(&ctx, &current_mode);
            break;
        case QUIT:
            handle_quit(&ctx);
            break;
        default:
            printf("Unknown opcode received: %d\n", rxBuffer[0]);
            break;
        }
    }

    close(server_sock);
    return 0;
}
