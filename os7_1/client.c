#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512
#define PORT 8080
#define ERROR -1
#define SUCCESS 1
#define TRUE 1

int main() {
    struct sockaddr_in server_addr;
    const char *server_ip = "127.0.0.1"; //локальный IP-адрес
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == ERROR) {
        perror("socket creation failed\n");
        return ERROR;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    int bin_server_ip = inet_pton(AF_INET, server_ip, &server_addr.sin_addr);
    if (bin_server_ip != SUCCESS) {
        perror("invalid address / address not supported\n");
        close(sockfd);
        return ERROR;
    }
    printf("UDP client connected to %s:%d\n", server_ip, PORT);
    while (TRUE) {
        printf("Enter message to send (or 'exit' to quit): ");
        char buffer[BUFFER_SIZE] = {0};
        char* fgets_ret = fgets(buffer, BUFFER_SIZE, stdin);
        if (fgets_ret == NULL) break;
        if (strcmp(buffer, "exit\n") == 0)
            break;
        ssize_t send_len = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *)&server_addr, sizeof(server_addr));
        if (send_len == ERROR) {
            perror("sendto failed\n");
            continue;
        }
        printf("Message sent to server\n");
        socklen_t server_len = sizeof(server_addr);
        memset((void*)buffer, 0, BUFFER_SIZE);
        ssize_t recv_len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&server_addr, &server_len);
        if (recv_len == ERROR) {
            perror("recvfrom failed\n");
            continue;
        }
        printf("Received from server: %s\n", buffer);
    }
    close(sockfd);
    return 0;
}
