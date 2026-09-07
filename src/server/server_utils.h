#ifndef SERVER_UTILS
#define SERVER_UTILS
#include "commons/commons.h"

Status recv_file_data(int fd, data_packet *pkt, int sock_fd,
                      struct sockaddr_in *server_addr);
#endif
