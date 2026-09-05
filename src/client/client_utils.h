#ifndef CLIENT_UTILS
#define CLIENT_UTILS
// client utility functions
#include "commons/commons.h"
Status validate_and_set_connection(char *cmd_buffer, int sock_fd,
                                   struct sockaddr_in *server_addr);
Status get_file(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr);
Status put_file(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr);
void quit(struct sockaddr_in *server_addr);
#endif
