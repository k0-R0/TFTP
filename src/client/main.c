// the main function for the client
//
// the client will make 4 kinds of requests
// 1. request a file from the server
// 2. send a file to the server
// 3. change the operation mode
// 4. quit the application and disconnect from the server
#include "client/client_utils.h"
#include "commons/commons.h"
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>

void print_menu(void) {
    printf("Menu:\n1. connect <server_ip>\n2. get <file_name>\n3. put "
           "<file_name>\n4. mode <op>\n5. quit\n");
}

Opcode get_operation(const char *cmd_buffer) {
    if (strncmp(cmd_buffer, "connect", 7) == 0)
        return CONNECT;
    else if (strncmp(cmd_buffer, "get", 3) == 0)
        return GET;
    else if (strncmp(cmd_buffer, "put", 3) == 0)
        return PUT;
    else if (strncmp(cmd_buffer, "quit", 4) == 0)
        return QUIT;
    return HELP;
}

int main() {
    // display the menu in an infinite loop
    char cmd_buffer[100];
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = SERVER_PORT;
    while (1) {
        print_menu();
        fgets(cmd_buffer, 100, stdin);
        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';
        if (strncmp(cmd_buffer, "help", 4) == 0)
            continue;
        Opcode op = get_operation(cmd_buffer);
        printf("op code is %d\n", op);
        switch (op) {
        case CONNECT:
            validate_and_set_connection(cmd_buffer, &server_addr);
            break;
        case GET:
            get_file(cmd_buffer, &server_addr);
            break;
        case PUT:
            put_file(cmd_buffer, &server_addr);
            break;
        case QUIT:
            quit(&server_addr);
            break;
        default:
            break;
        }
    }
}
