#ifndef CLIENT_UTILS_H
#define CLIENT_UTILS_H

#include "commons/commons.h"

Status validate_and_set_ip(char *ipstr, struct in_addr *addr);
Status connect_to_server(char *cmd_buffer, int sock_fd,
                         struct sockaddr_in *server_addr);
Status download_files(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr,
                      TransferMode mode);
Status upload_files(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr,
                    TransferMode mode);
Status set_transfer_mode(char *cmd_buffer, int sock_fd, struct sockaddr_in *server_addr,
                         TransferMode *client_mode);
void disconnect_and_quit(int sock_fd, struct sockaddr_in *server_addr);

#endif
