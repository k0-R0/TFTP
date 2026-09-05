#include "commons/commons.h"
#include "commons/logs.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static char *strip_command(char *cmd_buffer) {
    char prev = ' ';
    int i = 0;
    int word_count = 0;
    while (cmd_buffer[i] && word_count < 2) {
        if (!isspace(cmd_buffer[i]) && isspace(prev)) {
            word_count++;
        }
        prev = cmd_buffer[i];
        i++;
    }
    printf("%s\n", cmd_buffer + i - 1);
    return cmd_buffer + i - 1; // correcting i that went ahead by a character
}

Status validate_and_set_ip(char *ipstr, struct in_addr *addr) {
    if (inet_pton(AF_INET, ipstr, addr) == 0) {
        return FAILURE;
    }
    return SUCCESS;
}

Status validate_and_set_connection(char *cmd_buffer, int sock_fd,
                                   struct sockaddr_in *server_addr) {
    // validate ip address
    char *local_cmd_buffer = malloc((strlen(cmd_buffer) + 1) * sizeof(char));
    strcpy(local_cmd_buffer, cmd_buffer);
    char *ipstr = strip_command(local_cmd_buffer);
    // store ip address into buffer
    // convert ipstr to sock_addr_in.s_addr.in_addr
    if (validate_and_set_ip(ipstr, &server_addr->sin_addr) == FAILURE) {
        ERROR_INVALID_IP(ipstr);
        return FAILURE;
    }
    free(local_cmd_buffer);
    // connect to server
    if (sendto(sock_fd, cmd_buffer, strlen(cmd_buffer) + 1, 0,
               (struct sockaddr *)server_addr, sizeof(*server_addr)) == -1) {
        ERROR_SERVER_CONNECT();
        perror(NULL);
    }
    return SUCCESS;
}
Status get_file(char *cmd_buffer, struct sockaddr_in *server_addr) {
    return SUCCESS;
}
Status put_file(char *cmd_buffer, struct sockaddr_in *server_addr) {
    return SUCCESS;
}
void quit(struct sockaddr_in *server_addr) { return; }
