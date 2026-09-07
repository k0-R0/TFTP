#ifndef SERVER_UTILS
#define SERVER_UTILS
#include "commons/commons.h"

Status recv_file_data(int fd, int sock_fd, struct sockaddr_in *server_addr);
#endif
