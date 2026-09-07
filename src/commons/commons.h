#ifndef COMMONS_H
#define COMMONS_H
// header to declare the packet structure
// we need 3 kinds of packets -
// connection request packet to see if server is busy
// ack packet to acknowledge from server side
// command packet to perform 1 of the 4 operations
// file packet that has 512 bytes of file data
// ack packets for file and command
// size etc
// operation mode ENUM
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define DATA_BLOCKSIZE 512

typedef enum { CONNECT, GET, PUT, QUIT, ACK, ERROR, DATA, MODE, HELP } Opcode;
typedef enum { SUCCESS, FAILURE } Status;

typedef struct {
    char opcode;
    int block_num;
    int data_len;
    char data[DATA_BLOCKSIZE];
} data_packet;

typedef struct {
    int opcode;
    int block_num;
    char ack;
} ack_packet;

typedef struct {
    int opcode;
    char data[DATA_BLOCKSIZE];
} cmd_packet;

Status recv_file_data(int fd, int sock_fd, struct sockaddr_in *client_addr);
Status recv_file_block(data_packet *pkt, int sock_fd,
                       struct sockaddr_in *client_addr);
Status send_file_data(int fd, int sock_fd, struct sockaddr_in *server_addr);
Status send_file_block(data_packet *pkt, int sock_fd,
                       struct sockaddr_in *server_addr);
#endif
