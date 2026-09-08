#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// Uniform Error Logging Format:
// [ERROR] <Context> : <Specific explanatory message>

// Socket & Network Setup Errors
#define ERROR_INVALID_IP(ipstr)                                                \
    fprintf(stderr, "[ERROR] Network : Invalid IP address '%s'\n", (ipstr))
#define ERROR_SOCK_CREATE()                                                    \
    fprintf(stderr, "[ERROR] Socket : Creation failed\n")
#define ERROR_BIND_FAILED(port)                                                \
    fprintf(stderr, "[ERROR] Socket : Bind failed on port %d\n", (port))
#define ERROR_SERVER_LISTENER()                                                \
    fprintf(stderr, "[ERROR] Server Listener : Failed to receive incoming request\n")

// Connection Handshake Errors
#define ERROR_CONNECT_SEND(ip, port)                                           \
    fprintf(stderr, "[ERROR] Connect : Failed to send request to %s:%hu\n",    \
            (ip), (port))
#define ERROR_CONNECT_ACK_RECV()                                               \
    fprintf(stderr, "[ERROR] Connect : ACK not received from server\n")

// Command Errors
#define ERROR_CMD_SEND(cmd, ip, port)                                          \
    fprintf(stderr, "[ERROR] Command : Failed to send '%s' request to %s:%hu\n",\
            (cmd), (ip), (port))
#define ERROR_CMD_ACK_RECV(cmd)                                                \
    fprintf(stderr, "[ERROR] Command : ACK not received for '%s' request\n",    \
            (cmd))

// File I/O Errors
#define ERROR_FILE_OPEN(filename)                                              \
    fprintf(stderr, "[ERROR] File I/O : Failed to open file '%s'\n", (filename))
#define ERROR_FILE_BLOCK_READ(block_num)                                       \
    fprintf(stderr, "[ERROR] File I/O : Failed to read data block %d from file\n",\
            (block_num))
#define ERROR_FILE_BLOCK_WRITE(block_num)                                      \
    fprintf(stderr, "[ERROR] File I/O : Failed to write data block %d to file\n", \
            (block_num))

// Data Transfer (Packet) Errors
#define ERROR_DATA_BLOCK_SEND(block_num)                                       \
    fprintf(stderr, "[ERROR] Transfer : Failed to send DATA block %d\n",       \
            (block_num))
#define ERROR_DATA_BLOCK_RECV(block_num)                                       \
    fprintf(stderr, "[ERROR] Transfer : Failed to receive DATA block %d\n",    \
            (block_num))
#define ERROR_DATA_ACK_RECV(block_num)                                         \
    fprintf(stderr, "[ERROR] Transfer : ACK not received for DATA block %d\n", \
            (block_num))
#define ERROR_DATA_ACK_SEND(block_num)                                         \
    fprintf(stderr, "[ERROR] Transfer : Failed to send ACK for DATA block %d\n",\
            (block_num))

// Backward-compatibility aliases
#define ERROR_SERVER_CONNECT()                                                 \
    fprintf(stderr, "[ERROR] Connect : Operation failed\n")
#define ERROR_ACK_RECV()                                                       \
    fprintf(stderr, "[ERROR] Transfer : ACK not received\n")
#define ERROR_FILE_READ_FAILED(file_name) ERROR_FILE_OPEN(file_name)
#define ERROR_FILE_BLOCK_READ_FAILED(block_num) ERROR_FILE_BLOCK_READ(block_num)

#endif
