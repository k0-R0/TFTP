#include "commons/commons.h"
#include "commons/logs.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
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
    printf("%s\n", cmd_buffer + i - 1);
    return cmd_buffer + i - 1; // correcting i that went ahead by a character
}

Status validate_and_set_ip(char *ipstr, struct in_addr *addr) {
    if (inet_pton(AF_INET, ipstr, addr) == 0) {
        return FAILURE;
    }
    return SUCCESS;
}

Status validate_and_set_connection(char *cmd_buffer, int sock_fd,
                                   struct sockaddr_in *server_addr) {
    // validate ip address
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *ipstr = strip_command(local_cmd_buffer);
    // store ip address into buffer
    // convert ipstr to sock_addr_in.s_addr.in_addr
    if (validate_and_set_ip(ipstr, &server_addr->sin_addr) == FAILURE) {
        ERROR_INVALID_IP(ipstr);
        return FAILURE;
    }
    packet pkt;
    pkt.opcode = CONNECT;
    strcpy(pkt.data, ipstr);
    free(local_cmd_buffer);
    // connect to server
    if (sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_SERVER_CONNECT();
        perror(NULL);
    }
    // check for ack
    packet ack;
    socklen_t len = sizeof(ack);
    ssize_t bytes = recvfrom(sock_fd, &ack, sizeof(ack), 0,
                             (struct sockaddr *)server_addr, &len);
    if (bytes < 0) {
        ERROR_ACK_RECV();
        perror(NULL);
    }
    if (ack.opcode == ACK)
        printf("Sender IP : %s\nSender Port : %hu\nData : %s\n",
               inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port),
               ack.data);
    return SUCCESS;
}
Status send_file_data(int fd, packet *pkt, int sock_fd,
                      struct sockaddr_in *server_addr) {
    ssize_t bytes_read;
    pkt->block_num++;
    pkt->opcode = DATA;
    if ((bytes_read = read(fd, pkt->data, sizeof(pkt->data))) < 0) {
        ERROR_FILE_BLOCK_READ_FAILED(pkt->block_num);
        return FAILURE;
    }
    pkt->data_len = bytes_read;
    if (sendto(sock_fd, pkt, sizeof(*pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_SERVER_CONNECT();
        return FAILURE;
    }
    return SUCCESS;
}
Status get_file(char *cmd_buffer, int sock_fd,
                struct sockaddr_in *server_addr) {
    // set opcode
    packet pkt;
    pkt.opcode = GET;
    // get file names list pass it to data
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *files = strip_command(local_cmd_buffer);
    strcpy(pkt.data, files);
    // send packet
    if (sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_SERVER_CONNECT();
        perror(NULL);
    }
    // wait for ack
    packet ack;
    socklen_t len = sizeof(ack);
    ssize_t bytes = recvfrom(sock_fd, &ack, sizeof(ack), 0,
                             (struct sockaddr *)server_addr, &len);
    if (bytes < 0) {
        ERROR_ACK_RECV();
        perror(NULL);
    }
    if (ack.opcode == ACK)
        printf("Sender IP : %s\nSender Port : %hu\nData : %s\n",
               inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port),
               ack.data);
    // if ack data is error then display error
    // else create a copy of the file and then retrieve data block by block
    return SUCCESS;
}
Status put_file(char *cmd_buffer, int sock_fd,
                struct sockaddr_in *server_addr) {
    // set opcode
    packet pkt;
    pkt.opcode = PUT;
    // get file names list pass it to data
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *files = strip_command(local_cmd_buffer);
    strcpy(pkt.data, files);
    // send packet
    if (sendto(sock_fd, &pkt, sizeof(pkt), 0, (struct sockaddr *)server_addr,
               sizeof(*server_addr)) == -1) {
        ERROR_SERVER_CONNECT();
        perror(NULL);
    }
    // wait for ack
    packet ack;
    socklen_t len = sizeof(ack);
    ssize_t bytes = recvfrom(sock_fd, &ack, sizeof(ack), 0,
                             (struct sockaddr *)server_addr, &len);
    if (bytes < 0) {
        ERROR_ACK_RECV();
        perror(NULL);
    }
    if (ack.opcode == ACK) {
        printf("Sender IP : %s\nSender Port : %hu\nData : %s\n",
               inet_ntoa(server_addr->sin_addr), ntohs(server_addr->sin_port),
               ack.data);
        int file_fd = open(files, O_RDONLY);
        if (send_file_data(file_fd, &pkt, sock_fd, server_addr) == FAILURE) {
            printf("File send failed");
            return FAILURE;
        }
    }
    // if ack data is error then display error
    // else create a copy of the file and then retrieve data block by block
    return SUCCESS;
}
void quit(struct sockaddr_in *server_addr) { return; }
