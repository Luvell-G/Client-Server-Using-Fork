#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT 8080
#define BUF_SIZE 1024

void reap_children(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int client_sock) {
    char buffer[BUF_SIZE];
    int n;

    while (1) {
        memset(buffer, 0, BUF_SIZE);
        n = read(client_sock, buffer, BUF_SIZE - 1);
        if (n <= 0) break;

        printf("Received: %s", buffer);
        write(client_sock, buffer, strlen(buffer));
    }

    close(client_sock);
    exit(0);
}

int main() {
    int listen_sock, client_sock;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t clilen;

    signal(SIGCHLD, reap_children);

    listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        perror("Socket failed");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    bind(listen_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    listen(listen_sock, 5);

    printf("Server running on port %d...\n", PORT);

    while (1) {
        clilen = sizeof(cli_addr);
        client_sock = accept(listen_sock, (struct sockaddr *)&cli_addr, &clilen);

        if (fork() == 0) {
            close(listen_sock);
            handle_client(client_sock);
        } else {
            close(client_sock);
        }
    }

    close(listen_sock);
    return 0;
}
