#include "client_utils.h"
#include "commons/commons.h"
#include "commons/logs.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>

static char *strip_command(char *cmd_buffer) {
    char prev = ' ';
    int i = 0;
    int word_count = 0;
    while (cmd_buffer[i] && word_count < 2) {
        if (!isspace(cmd_buffer[i]) && isspace(prev)) {
            word_count++;
        }
        prev = cmd_buffer[i];
        i++;
    }
    return cmd_buffer + i - 1; // correcting i that went ahead by a character
}

Status validate_and_set_ip(char *ipstr, struct in_addr *addr) {
    if (inet_pton(AF_INET, ipstr, addr) == 0) {
        return FAILURE;
    }
    return SUCCESS;
}

Status connect_to_server(char *cmd_buffer, int sock_fd,
                         struct sockaddr_in *server_addr) {
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *ipstr = strip_command(local_cmd_buffer);

    if (validate_and_set_ip(ipstr, &server_addr->sin_addr) == FAILURE) {
        ERROR_INVALID_IP(ipstr);
        free(local_cmd_buffer);
        return FAILURE;
    }

    cmd_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.opcode = CONNECT;
    strncpy(pkt.data, ipstr, sizeof(pkt.data) - 1);
    free(local_cmd_buffer);

    // connect to server
    if (sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_CONNECT_SEND(inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port));
        perror("sendto");
        return FAILURE;
    }

    // check for ack
    ack_packet ack;
    struct sockaddr_in from_addr;
    socklen_t len = sizeof(from_addr);
    ssize_t bytes = recvfrom(sock_fd, &ack, sizeof(ack), 0,
                             (struct sockaddr *)&from_addr, &len);
    if (bytes < 0) {
        ERROR_CONNECT_ACK_RECV();
        return FAILURE;
    }

    if (ack.opcode == ACK) {
        printf("Connected to %s:%hu -> %s\n",
               inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port),
               ack.message);
    }
    return SUCCESS;
}

Status download_files(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr,
                      TransferMode mode) {
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *files_str = strip_command(local_cmd_buffer);

    cmd_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.opcode = GET;
    strncpy(pkt.data, files_str, sizeof(pkt.data) - 1);

    // Send initial request with all requested files
    if (sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_CMD_SEND("GET", inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port));
        perror("sendto");
        free(local_cmd_buffer);
        return FAILURE;
    }

    // Parse the file list to receive each file sequentially
    char **files = parse_file_list(files_str);
    if (!files) {
        free(local_cmd_buffer);
        return FAILURE;
    }

    struct sockaddr_in worker_addr = *server_addr;
    for (int i = 0; files[i] != NULL; i++) {
        ack_packet ack;
        socklen_t len = sizeof(worker_addr);
        ssize_t bytes = recvfrom(sock_fd, &ack, sizeof(ack), 0,
                                 (struct sockaddr *)&worker_addr, &len);
        if (bytes < 0) {
            ERROR_CMD_ACK_RECV("GET");
            break;
        }

        if (ack.opcode == ACK && ack.ack == 1) {
            printf("Downloading [%s] from %s:%hu...\n", ack.message,
                   inet_ntoa(worker_addr.sin_addr), ntohs(worker_addr.sin_port));

            char file_path[256];
            snprintf(file_path, sizeof(file_path), "client_downloads/%s", ack.message);
            int file_fd = open(file_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file_fd < 0) {
                ERROR_FILE_OPEN(file_path);
                continue;
            }

            FileContext ctx = {
                .sock_fd = sock_fd,
                .dest_addr = worker_addr,
                .mode = mode
            };

            recv_file_data(file_fd, &ctx);
            close(file_fd);
            printf("Saved to %s successfully.\n", file_path);
        } else if (ack.opcode == ACK && ack.ack == 0) {
            printf("Server rejected file [%s]: %s\n", files[i], ack.message);
        }
    }

    free_file_list(files);
    free(local_cmd_buffer);
    return SUCCESS;
}

Status upload_files(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr,
                    TransferMode mode) {
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *files_str = strip_command(local_cmd_buffer);

    cmd_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.opcode = PUT;
    strncpy(pkt.data, files_str, sizeof(pkt.data) - 1);

    // Send initial request with all files to upload
    if (sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_CMD_SEND("PUT", inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port));
        perror("sendto");
        free(local_cmd_buffer);
        return FAILURE;
    }

    // Parse the file list to send each file sequentially
    char **files = parse_file_list(files_str);
    if (!files) {
        free(local_cmd_buffer);
        return FAILURE;
    }

    struct sockaddr_in worker_addr = *server_addr;
    for (int i = 0; files[i] != NULL; i++) {
        ack_packet ack;
        socklen_t len = sizeof(worker_addr);
        ssize_t bytes = recvfrom(sock_fd, &ack, sizeof(ack), 0,
                                 (struct sockaddr *)&worker_addr, &len);
        if (bytes < 0) {
            ERROR_CMD_ACK_RECV("PUT");
            break;
        }

        if (ack.opcode == ACK && ack.ack == 1) {
            printf("Uploading [%s] to %s:%hu...\n", ack.message,
                   inet_ntoa(worker_addr.sin_addr), ntohs(worker_addr.sin_port));

            int file_fd = open(files[i], O_RDONLY);
            if (file_fd < 0) {
                ERROR_FILE_OPEN(files[i]);
                continue;
            }

            FileContext ctx = {
                .sock_fd = sock_fd,
                .dest_addr = worker_addr,
                .mode = mode
            };

            if (send_file_data(file_fd, &ctx) == FAILURE) {
                printf("Failed sending file: %s\n", files[i]);
                close(file_fd);
                break;
            }
            close(file_fd);
            printf("Uploaded %s successfully.\n", files[i]);
        } else if (ack.opcode == ACK && ack.ack == 0) {
            printf("Server rejected file [%s]: %s\n", files[i], ack.message);
        }
    }

    free_file_list(files);
    free(local_cmd_buffer);
    return SUCCESS;
}

Status set_transfer_mode(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr,
                         TransferMode *client_mode) {
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *mode_str = strip_command(local_cmd_buffer);

    cmd_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.opcode = MODE;
    strncpy(pkt.data, mode_str, sizeof(pkt.data) - 1);

    if (sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_CMD_SEND("MODE", inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port));
        perror("sendto");
        free(local_cmd_buffer);
        return FAILURE;
    }

    ack_packet ack;
    struct sockaddr_in from_addr;
    socklen_t len = sizeof(from_addr);
    ssize_t bytes = recvfrom(sock_fd, &ack, sizeof(ack), 0,
                             (struct sockaddr *)&from_addr, &len);
    if (bytes < 0) {
        ERROR_CMD_ACK_RECV("MODE");
        free(local_cmd_buffer);
        return FAILURE;
    }

    if (ack.opcode == ACK && ack.ack == 1) {
        printf("Server: %s\n", ack.message);
        if (strcasecmp(mode_str, "byte") == 0) {
            *client_mode = MODE_BYTE;
        } else if (strcasecmp(mode_str, "mail") == 0) {
            *client_mode = MODE_MAIL;
        } else if (strcasecmp(mode_str, "octet") == 0) {
            *client_mode = MODE_OCTET;
        }
    } else {
        printf("Mode change failed: %s\n", ack.message);
    }

    free(local_cmd_buffer);
    return SUCCESS;
}

void disconnect_and_quit(int sock_fd, struct sockaddr_in *server_addr) {
    cmd_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.opcode = QUIT;

    sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
           sizeof(*server_addr));

    ack_packet ack;
    struct sockaddr_in from_addr;
    socklen_t len = sizeof(from_addr);
    if (recvfrom(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr *)&from_addr,
                 &len) > 0) {
        if (ack.opcode == ACK) {
            printf("Server: %s\n", ack.message);
        }
    }
}
