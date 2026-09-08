// the main function for the server
#include "commons/commons.h"
#include "commons/logs.h"
#include "server_utils.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#define MAX_THREADS 10
static sem_t thread_sem;

void *handle_get_thread(void *args) {
    FileContext *ctx = (FileContext *)args;
    int thread_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (thread_sock >= 0) {
        ctx->sock_fd = thread_sock;
        handle_get(ctx);
        close(thread_sock);
    }
    free(ctx);
    sem_post(&thread_sem);
    return NULL;
}

void *handle_put_thread(void *args) {
    FileContext *ctx = (FileContext *)args;
    int thread_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (thread_sock >= 0) {
        ctx->sock_fd = thread_sock;
        handle_put(ctx);
        close(thread_sock);
    }
    free(ctx);
    sem_post(&thread_sem);
    return NULL;
}

int main() {
    int server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock < 0) {
        ERROR_SOCK_CREATE();
        perror(NULL);
        return FAILURE;
    }
    // initializing the semaphore
    sem_init(&thread_sem, 0, MAX_THREADS);

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

        FileContext ctx = {.sock_fd = server_sock,
                           .dest_addr = client_addr,
                           .pkt = *(cmd_packet *)rxBuffer,
                           .mode = current_mode};

        switch (rxBuffer[0]) {
        case CONNECT:
            handle_connect(&ctx);
            break;
        case GET: {
            if (sem_trywait(&thread_sem) != 0) {
                ack_packet busy_ack = {
                    .opcode = ACK,
                    .ack = 0,
                    .ack_op = GET,
                    .message = "Server is busy , please try again later"};
                sendto(server_sock, &busy_ack, sizeof(busy_ack), 0,
                       (struct sockaddr *)&client_addr, sizeof(client_addr));
                break;
            }
            FileContext *thread_ctx = malloc(sizeof(FileContext));
            if (!thread_ctx) {
                sem_post(&thread_sem);
                break;
            }
            thread_ctx->dest_addr = client_addr;
            thread_ctx->mode = current_mode;
            thread_ctx->pkt = *(cmd_packet *)rxBuffer;
            pthread_t thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            if (pthread_create(&thread, &attr, handle_get_thread, thread_ctx) !=
                0) {
                free(thread_ctx);
                sem_post(&thread_sem);
            }
            pthread_attr_destroy(&attr);
            break;
        }
        case PUT: {
            if (sem_trywait(&thread_sem) != 0) {
                ack_packet busy_ack = {
                    .opcode = ACK,
                    .ack = 0,
                    .ack_op = PUT,
                    .message = "Server is busy , please try again later"};
                sendto(server_sock, &busy_ack, sizeof(busy_ack), 0,
                       (struct sockaddr *)&client_addr, sizeof(client_addr));
                break;
            }
            FileContext *thread_ctx = malloc(sizeof(FileContext));
            if (!thread_ctx) {
                sem_post(&thread_sem);
                break;
            }
            thread_ctx->dest_addr = client_addr;
            thread_ctx->mode = current_mode;
            thread_ctx->pkt = *(cmd_packet *)rxBuffer;
            pthread_t thread;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
            if (pthread_create(&thread, &attr, handle_put_thread, thread_ctx) !=
                0) {
                free(thread_ctx);
                sem_post(&thread_sem);
            }
            pthread_attr_destroy(&attr);
            break;
        }
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
