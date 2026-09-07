#include "commons/commons.h"
#include "commons/logs.h"

Status recv_file_data(int fd, packet *pkt, int sock_fd,
                      struct sockaddr_in *client_addr) {
    ssize_t bytes_written;
    pkt->block_num++;
    pkt->opcode = DATA;
    socklen_t len = sizeof(*client_addr);
    if (recvfrom(sock_fd, pkt, sizeof(*pkt), 0, (struct sockaddr *)client_addr,
                 &len) == -1) {
        ERROR_SERVER_CONNECT();
        return FAILURE;
    }
    if ((bytes_written = write(fd, pkt->data, pkt->data_len)) < 0) {
        ERROR_FILE_BLOCK_READ_FAILED(pkt->block_num);
        perror(NULL);
        return FAILURE;
    }
    return SUCCESS;
}
