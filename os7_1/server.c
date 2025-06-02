#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 512
#define PORT 8080
#define ERROR -1
#define TRUE 1

int main() {
    struct sockaddr_in server_addr, client_addr;
    int sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); 
    if (sockfd < 0) {
        perror("Socket creation failed.\n");
        return ERROR;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;//Указывает семейство адресов.
    server_addr.sin_addr.s_addr = INADDR_ANY;// 0.0.0.0
    server_addr.sin_port = htons(PORT);// Порт 8080 в сетевом порядке байт
    int check_error = bind(sockfd, (const struct sockaddr*)&server_addr, sizeof(server_addr));
    if (check_error == ERROR) {
        perror("bind failed.\n");
        close(sockfd);
        return ERROR;
    }
    printf("UDP echo server is running on port %d\n", PORT);
    while (TRUE) {
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t client_len = sizeof(client_addr);
        char buffer[BUFFER_SIZE + 1] = {0};
        ssize_t recv_len = recvfrom(sockfd, (char*)buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);
        if (recv_len == ERROR) {
            perror("recvfrom failed.\n");
            continue;
        }
        buffer[recv_len] = '\0';
        char* client_ip = inet_ntoa(client_addr.sin_addr);
        printf("Received from %s:%d - %s\n", client_ip, ntohs(client_addr.sin_port), buffer);
        //отправляет клиенту с адресом client_addr
        ssize_t send_len = sendto(sockfd, buffer, recv_len, 0, (const struct sockaddr *)&client_addr, client_len);
        if (send_len == ERROR) {
            perror("sendto failed.\n");
            continue;
        }
        printf("Echoed back to client\n");
    }
    close(sockfd);
    return 0;
}
