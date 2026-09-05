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
typedef enum { CONNECT, GET, PUT, QUIT, HELP } Opcode;
typedef enum { SUCCESS, FAILURE } Status;
#endif
