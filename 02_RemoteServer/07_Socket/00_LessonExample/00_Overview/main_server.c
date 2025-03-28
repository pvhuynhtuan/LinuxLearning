#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8088
#define LISTEN_BACKLOG 50

void main(int argc, char *argv[])
{
    int port_no, len, opt;
    int server_fd, new_socket_fd;
    struct sockaddr_in serv_addr, client_addr;
    char buffer[256];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        perror("Bind failed. Error");

    if (listen(server_fd, LISTEN_BACKLOG) == -1)
        printf("Listen failed\n");

    new_socket_fd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&len);

    if (new_socket_fd == -1)
        printf("accept failed\n");
    printf("client_addr.sin_addr.s_addr = %d\n",client_addr.sin_addr.s_addr);
    read(new_socket_fd, buffer, sizeof(buffer));
    printf("Message received: %s\n", buffer);
}