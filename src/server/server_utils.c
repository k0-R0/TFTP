#include "commons/commons.h"
#include "commons/logs.h"
#include <string.h>

Status recv_file_data(int fd, data_packet *pkt, int sock_fd,
                      struct sockaddr_in *client_addr) {
    ssize_t bytes_written;
    socklen_t len = sizeof(*client_addr);
    ack_packet ack;
    memset(&ack, 0, sizeof(ack));
    ack.opcode = ACK;
    if (recvfrom(sock_fd, pkt, sizeof(*pkt), 0, (struct sockaddr *)client_addr,
                 &len) == -1) {
        ERROR_SERVER_CONNECT();
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
    if ((bytes_written = write(fd, pkt->data, pkt->data_len)) < 0) {
        ERROR_FILE_BLOCK_READ_FAILED(pkt->block_num);
        perror(NULL);
        return FAILURE;
    }
    return SUCCESS;
}
