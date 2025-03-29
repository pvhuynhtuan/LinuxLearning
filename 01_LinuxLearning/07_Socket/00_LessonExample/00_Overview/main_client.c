#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORT 8088

void main(int argc, char *argv[])
{
    int port_no, len, opt;
    int client_fd;
    struct sockaddr_in serv_addr;
    char *buffer = "Hello from client!";

    client_fd = socket(AF_INET, SOCK_STREAM, 0);

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(client_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
        printf("Connect failed\n");

    write(client_fd, buffer, strlen(buffer) + 1);
}