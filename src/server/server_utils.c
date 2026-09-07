#include "commons/commons.h"
#include "commons/logs.h"
#include <string.h>

Status recv_file_block(data_packet *pkt, int sock_fd,
                       struct sockaddr_in *client_addr) {
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
    return SUCCESS;
}

Status recv_file_data(int fd, int sock_fd, struct sockaddr_in *client_addr) {
    data_packet pkt;
    memset(&pkt, 0, sizeof(pkt));
    while (1) {
        if (recv_file_block(&pkt, sock_fd, client_addr) == FAILURE) {
            ERROR_FILE_BLOCK_READ_FAILED(pkt.block_num);
            return FAILURE;
        }
        ssize_t bytes_written;
        if ((bytes_written = write(fd, pkt.data, pkt.data_len)) < 0) {
            ERROR_FILE_BLOCK_READ_FAILED(pkt.block_num);
            perror(NULL);
            return FAILURE;
        }
        if (pkt.data_len < DATA_BLOCKSIZE)
            break;
    }
    return SUCCESS;
}
