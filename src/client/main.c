// the main function for the client
#include "client/client_utils.h"
#include "commons/commons.h"
#include "commons/logs.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void print_menu(void) {
    printf("\n==================== TFTP Client Menu ====================\n"
           "Commands:\n"
           "  connect <server_ip>      Connect to server (e.g. connect 127.0.0.1)\n"
           "  get <file1> [file2 ...]  Download file(s) to client_downloads/\n"
           "  put <file1> [file2 ...]  Upload file(s) to server\n"
           "  mode <octet|byte|mail>   Set transfer mode:\n"
           "                             octet: binary in 512-byte blocks (default)\n"
           "                             byte : single-byte blocks (1 byte/pkt)\n"
           "                             mail : 512-byte text mode (escapes \\n to \\n\\r)\n"
           "  help                     Show this menu\n"
           "  quit                     Disconnect from server and exit\n"
           "==========================================================\n\n");
}

Opcode get_operation(const char *cmd_buffer) {
    if (strncmp(cmd_buffer, "connect", 7) == 0)
        return CONNECT;
    else if (strncmp(cmd_buffer, "get", 3) == 0)
        return GET;
    else if (strncmp(cmd_buffer, "put", 3) == 0)
        return PUT;
    else if (strncmp(cmd_buffer, "mode", 4) == 0)
        return MODE;
    else if (strncmp(cmd_buffer, "quit", 4) == 0)
        return QUIT;
    return HELP;
}

int main() {
    char cmd_buffer[100];
    int client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock < 0) {
        ERROR_SOCK_CREATE();
        perror(NULL);
        return FAILURE;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    TransferMode current_mode = MODE_OCTET;

    print_menu();
    while (1) {
        printf("tftp> ");
        fflush(stdout);
        if (!fgets(cmd_buffer, sizeof(cmd_buffer), stdin))
            break;
        cmd_buffer[strcspn(cmd_buffer, "\n")] = '\0';
        if (strlen(cmd_buffer) == 0)
            continue;
        if (strncmp(cmd_buffer, "help", 4) == 0) {
            print_menu();
            continue;
        }

        Opcode op = get_operation(cmd_buffer);
        switch (op) {
        case CONNECT:
            connect_to_server(cmd_buffer, client_sock, &server_addr);
            break;
        case GET:
            download_files(cmd_buffer, client_sock, &server_addr, current_mode);
            break;
        case PUT:
            upload_files(cmd_buffer, client_sock, &server_addr, current_mode);
            break;
        case MODE:
            set_transfer_mode(cmd_buffer, client_sock, &server_addr, &current_mode);
            break;
        case QUIT:
            disconnect_and_quit(client_sock, &server_addr);
            close(client_sock);
            return 0;
        default:
            print_menu();
            break;
        }
    }

    close(client_sock);
    return 0;
}
