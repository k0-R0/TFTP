#ifndef LOG_H
#define LOG_H
#include <stdio.h>
// Information logs
// Error logs
#define ERROR_INVALID_IP(ipstr) printf("Invalid IP address : %s\n", ipstr)
#define ERROR_SOCK_CREATE() printf("Socket creation failed\n")
#define ERROR_SERVER_CONNECT() printf("Connect failed")
#endif
