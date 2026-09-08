#include "commons.h"
#include "logs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char **parse_file_list(const char *files_str) {
    if (!files_str || !*files_str)
        return NULL;

    char *dup = strdup(files_str);
    if (!dup)
        return NULL;

    // Count non-empty tokens
    int count = 0;
    char *saveptr;
    char *token = strtok_r(dup, " \t\r\n", &saveptr);
    while (token) {
        count++;
        token = strtok_r(NULL, " \t\r\n", &saveptr);
    }
    free(dup);

    if (count == 0)
        return NULL;

    char **file_list = malloc(sizeof(char *) * (count + 1));
    if (!file_list)
        return NULL;

    dup = strdup(files_str);
    int idx = 0;
    token = strtok_r(dup, " \t\r\n", &saveptr);
    while (token) {
        file_list[idx++] = strdup(token);
        token = strtok_r(NULL, " \t\r\n", &saveptr);
    }
    file_list[idx] = NULL; // Sentinel
    free(dup);

    return file_list;
}

void free_file_list(char **files) {
    if (!files)
        return;
    for (int i = 0; files[i] != NULL; i++) {
        free(files[i]);
    }
    free(files);
}

Status send_file_block(data_packet *pkt, int sock_fd,
                       struct sockaddr_in *server_addr) {
    if (sendto(sock_fd, pkt, sizeof(*pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_DATA_BLOCK_SEND(pkt->block_num);
        return FAILURE;
    }
    ack_packet ack;
    if (recvfrom(sock_fd, &ack, sizeof(ack), 0, NULL, NULL) == -1) {
        ERROR_DATA_ACK_RECV(pkt->block_num);
        return FAILURE;
    }
    while (ack.opcode == ACK && ack.ack == 0) {
        if (sendto(sock_fd, pkt, sizeof(*pkt), 0,
                   (struct sockaddr *)server_addr,
                   sizeof(*server_addr)) == -1) {
            ERROR_DATA_BLOCK_SEND(pkt->block_num);
            return FAILURE;
        }
        if (recvfrom(sock_fd, &ack, sizeof(ack), 0, NULL, NULL) == -1) {
            ERROR_DATA_ACK_RECV(pkt->block_num);
            return FAILURE;
        }
    }
    return SUCCESS;
}

Status send_file_data(int fd, FileContext *ctx) {
    data_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    size_t block_size = (ctx->mode == MODE_BYTE) ? 1 : DATA_BLOCKSIZE;

    while (1) {
        ssize_t bytes_read = 0;
        pkt.block_num++;
        pkt.opcode = DATA;

        if (ctx->mode == MODE_MAIL) {
            int out_idx = 0;
            while (out_idx < DATA_BLOCKSIZE) {
                char ch;
                ssize_t r = read(fd, &ch, 1);
                if (r <= 0) {
                    break;
                }
                if (ch == '\n') {
                    if (out_idx + 2 > DATA_BLOCKSIZE) {
                        lseek(fd, -1, SEEK_CUR);
                        break;
                    }
                    pkt.data[out_idx++] = '\n';
                    pkt.data[out_idx++] = '\r';
                } else {
                    pkt.data[out_idx++] = ch;
                }
            }
            bytes_read = out_idx;
        } else {
            bytes_read = read(fd, pkt.data, block_size);
            if (bytes_read < 0) {
                ERROR_FILE_BLOCK_READ(pkt.block_num);
                return FAILURE;
            }
        }

        pkt.data_len = bytes_read;
        if (send_file_block(&pkt, ctx->sock_fd, &ctx->dest_addr) == FAILURE) {
            return FAILURE;
        }

        if ((size_t)bytes_read < block_size)
            break;
    }
    return SUCCESS;
}

Status recv_file_block(data_packet *pkt, int sock_fd,
                       struct sockaddr_in *client_addr) {
    socklen_t len = sizeof(*client_addr);
    ack_packet ack;
    memset(&ack, 0, sizeof(ack));
    ack.opcode = ACK;
    ack.ack_op = DATA;

    if (recvfrom(sock_fd, pkt, sizeof(*pkt), 0, (struct sockaddr *)client_addr,
                 &len) == -1) {
        ERROR_DATA_BLOCK_RECV(pkt->block_num);
        ack.block_num = pkt->block_num;
        ack.ack = 0;
        sendto(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr *)client_addr,
               len);
        return FAILURE;
    }

    // send ack for block
    ack.block_num = pkt->block_num;
    ack.ack = 1;
    sendto(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr *)client_addr, len);
    return SUCCESS;
}

Status recv_file_data(int fd, FileContext *ctx) {
    data_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    size_t block_size = (ctx->mode == MODE_BYTE) ? 1 : DATA_BLOCKSIZE;

    while (1) {
        if (recv_file_block(&pkt, ctx->sock_fd, &ctx->dest_addr) == FAILURE) {
            return FAILURE;
        }

        ssize_t bytes_written = 0;
        if (ctx->mode == MODE_MAIL) {
            char clean_buf[DATA_BLOCKSIZE];
            int write_idx = 0;
            for (int i = 0; i < pkt.data_len; i++) {
                if (pkt.data[i] == '\n' && i + 1 < pkt.data_len &&
                    pkt.data[i + 1] == '\r') {
                    clean_buf[write_idx++] = '\n';
                    i++; // skip '\r'
                } else {
                    clean_buf[write_idx++] = pkt.data[i];
                }
            }
            if (write_idx > 0) {
                bytes_written = write(fd, clean_buf, write_idx);
            }
        } else {
            if (pkt.data_len > 0) {
                bytes_written = write(fd, pkt.data, pkt.data_len);
            }
        }

        if (bytes_written < 0) {
            ERROR_FILE_BLOCK_WRITE(pkt.block_num);
            perror("write");
            return FAILURE;
        }

        if ((size_t)pkt.data_len < block_size)
            break;
    }
    return SUCCESS;
}
