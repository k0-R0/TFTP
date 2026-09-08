#ifndef COMMONS_H
#define COMMONS_H

#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define DATA_BLOCKSIZE 512

typedef enum { CONNECT, GET, PUT, QUIT, ACK, ERROR, DATA, MODE, HELP } Opcode;
typedef enum { SUCCESS, FAILURE } Status;

// 3 Transfer Modes
typedef enum {
    MODE_OCTET,  // Standard 512-byte binary transfer
    MODE_BYTE,   // Byte-by-byte transfer (1 byte blocks)
    MODE_MAIL    // 512-byte transfer, converting '\n' to '\n\r'
} TransferMode;

typedef struct {
    char opcode;
    int block_num;
    int data_len;
    char data[DATA_BLOCKSIZE];
} data_packet;

typedef struct {
    int opcode;
    int block_num;
    int ack_op;        // Opcode being acknowledged (CONNECT, GET, PUT, MODE, QUIT, DATA)
    char ack;          // 1 for success, 0 for failure
    char message[128]; // Filename or descriptive acknowledgment text
} ack_packet;

typedef struct {
    int opcode;
    char data[DATA_BLOCKSIZE];
} cmd_packet;

// Context holding socket, destination address, command packet, and transfer mode
typedef struct {
    int sock_fd;
    struct sockaddr_in dest_addr;
    cmd_packet pkt;
    TransferMode mode;
} FileContext;

// Filename string parsing helpers
char **parse_file_list(const char *files_str);
void free_file_list(char **files);

// Mode-aware file transfer functions using FileContext
Status send_file_data(int fd, FileContext *ctx);
Status send_file_block(data_packet *pkt, int sock_fd,
                       struct sockaddr_in *server_addr);

Status recv_file_data(int fd, FileContext *ctx);
Status recv_file_block(data_packet *pkt, int sock_fd,
                       struct sockaddr_in *client_addr);

#endif
