#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define BUF_SIZE 1024

int main() {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    while (1) {
        printf("Enter message: ");
        fgets(buffer, BUF_SIZE, stdin);

        write(sockfd, buffer, strlen(buffer));

        memset(buffer, 0, BUF_SIZE);
        read(sockfd, buffer, BUF_SIZE - 1);

        printf("Server replied: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
