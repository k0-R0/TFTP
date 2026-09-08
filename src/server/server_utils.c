#include "server_utils.h"
#include "commons/commons.h"
#include "commons/logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

Status handle_connect(FileContext *ctx) {
    ack_packet ack;
    memset(&ack, 0, sizeof(ack));
    ack.opcode = ACK;
    ack.ack_op = CONNECT;
    ack.ack = 1;
    strncpy(ack.message, "Connected successfully", sizeof(ack.message) - 1);

    printf("Client connected from %s:%hu (Data: %s)\n",
           inet_ntoa(ctx->dest_addr.sin_addr), ntohs(ctx->dest_addr.sin_port),
           ctx->pkt.data);

    sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
           (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
    return SUCCESS;
}

Status handle_get(FileContext *ctx) {
    char **files = parse_file_list(ctx->pkt.data);
    if (!files) {
        ack_packet ack;
        memset(&ack, 0, sizeof(ack));
        ack.opcode = ACK;
        ack.ack_op = GET;
        ack.ack = 0;
        strncpy(ack.message, "No file specified", sizeof(ack.message) - 1);
        sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
               (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
        return FAILURE;
    }

    for (int i = 0; files[i] != NULL; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "server_downloads/%s", files[i]);
        int file_fd = open(filepath, O_RDONLY);
        if (file_fd < 0) {
            perror(filepath);
            ack_packet ack;
            memset(&ack, 0, sizeof(ack));
            ack.opcode = ACK;
            ack.ack_op = GET;
            ack.ack = 0;
            snprintf(ack.message, sizeof(ack.message), "%s: Not found", files[i]);
            sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
               (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
            continue;
        }

        // Acknowledge GET with the specific filename being transferred
        ack_packet ack;
        memset(&ack, 0, sizeof(ack));
        ack.opcode = ACK;
        ack.ack_op = GET;
        ack.ack = 1;
        strncpy(ack.message, files[i], sizeof(ack.message) - 1);
        sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
               (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));

        printf("Sending file: %s to %s:%hu (mode: %d)\n", files[i],
               inet_ntoa(ctx->dest_addr.sin_addr),
               ntohs(ctx->dest_addr.sin_port), ctx->mode);

        if (send_file_data(file_fd, ctx) == FAILURE) {
            printf("Failed sending file: %s\n", files[i]);
            close(file_fd);
            break;
        }
        close(file_fd);
    }

    free_file_list(files);
    return SUCCESS;
}

Status handle_put(FileContext *ctx) {
    char **files = parse_file_list(ctx->pkt.data);
    if (!files) {
        ack_packet ack;
        memset(&ack, 0, sizeof(ack));
        ack.opcode = ACK;
        ack.ack_op = PUT;
        ack.ack = 0;
        strncpy(ack.message, "No file specified", sizeof(ack.message) - 1);
        sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
               (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
        return FAILURE;
    }

    for (int i = 0; files[i] != NULL; i++) {
        char filepath[256];
        snprintf(filepath, sizeof(filepath), "server_downloads/%s", files[i]);
        int file_fd = open(filepath, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (file_fd < 0) {
            perror(filepath);
            ack_packet ack;
            memset(&ack, 0, sizeof(ack));
            ack.opcode = ACK;
            ack.ack_op = PUT;
            ack.ack = 0;
            snprintf(ack.message, sizeof(ack.message), "%s: Open failed", files[i]);
            sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
                   (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
            continue;
        }

        // Acknowledge PUT with the specific filename being transferred
        ack_packet ack;
        memset(&ack, 0, sizeof(ack));
        ack.opcode = ACK;
        ack.ack_op = PUT;
        ack.ack = 1;
        strncpy(ack.message, files[i], sizeof(ack.message) - 1);
        sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
               (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));

        printf("Receiving file: %s from %s:%hu (mode: %d)\n", files[i],
               inet_ntoa(ctx->dest_addr.sin_addr),
               ntohs(ctx->dest_addr.sin_port), ctx->mode);

        if (recv_file_data(file_fd, ctx) == FAILURE) {
            printf("Failed receiving file: %s\n", files[i]);
            close(file_fd);
            break;
        }
        close(file_fd);
    }

    free_file_list(files);
    return SUCCESS;
}

Status handle_mode(FileContext *ctx, TransferMode *server_mode) {
    ack_packet ack;
    memset(&ack, 0, sizeof(ack));
    ack.opcode = ACK;
    ack.ack_op = MODE;
    ack.ack = 1;

    char mode_str[32] = {0};
    sscanf(ctx->pkt.data, "%31s", mode_str);

    if (strcasecmp(mode_str, "byte") == 0) {
        *server_mode = MODE_BYTE;
        strncpy(ack.message, "Mode set to byte", sizeof(ack.message) - 1);
    } else if (strcasecmp(mode_str, "mail") == 0) {
        *server_mode = MODE_MAIL;
        strncpy(ack.message, "Mode set to mail", sizeof(ack.message) - 1);
    } else if (strcasecmp(mode_str, "octet") == 0) {
        *server_mode = MODE_OCTET;
        strncpy(ack.message, "Mode set to octet", sizeof(ack.message) - 1);
    } else {
        ack.ack = 0;
        snprintf(ack.message, sizeof(ack.message), "Unknown mode: %s", mode_str);
    }

    printf("Mode change from %s:%hu -> %s\n",
           inet_ntoa(ctx->dest_addr.sin_addr), ntohs(ctx->dest_addr.sin_port),
           ack.message);

    sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
           (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
    return ack.ack ? SUCCESS : FAILURE;
}

Status handle_quit(FileContext *ctx) {
    ack_packet ack;
    memset(&ack, 0, sizeof(ack));
    ack.opcode = ACK;
    ack.ack_op = QUIT;
    ack.ack = 1;
    strncpy(ack.message, "Goodbye", sizeof(ack.message) - 1);

    printf("Client disconnected: %s:%hu\n",
           inet_ntoa(ctx->dest_addr.sin_addr), ntohs(ctx->dest_addr.sin_port));

    sendto(ctx->sock_fd, &ack, sizeof(ack), 0,
           (struct sockaddr *)&ctx->dest_addr, sizeof(ctx->dest_addr));
    return SUCCESS;
}
