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
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define DATA_BLOCKSIZE 512

typedef enum { CONNECT, GET, PUT, QUIT, ACK, DATA, MODE, HELP } Opcode;
typedef enum { SUCCESS, FAILURE } Status;

typedef struct {
    int opcode;
    int block_num;
    char data[DATA_BLOCKSIZE];
} packet;

#endif
