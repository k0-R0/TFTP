#ifndef CLIENT_UTILS
#define CLIENT_UTILS
// client utility functions
#include "commons/commons.h"
Status validate_and_set_connection(char *cmd_buffer, int *server_ip);
Status get_file(char *cmd_buffer, int *server_ip);
Status put_file(char *cmd_buffer, int *server_ip);
void quit(int *server_ip);
#endif
