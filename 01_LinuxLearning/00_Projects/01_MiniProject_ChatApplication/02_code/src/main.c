#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>

#define PORT 8088
#define LISTEN_BACKLOG 50

void CA_help();
void CA_myip();
void CA_myport();
void CA_connect(char *des, int port);
void CA_list();
void CA_terminate();
void CA_send();
void CA_exit();

void main(int argc, char *argv[])
{
    
    if (strcmp(argv[1], "help") == 0)
    {
        CA_help();
    }
    else if (strcmp(argv[1], "myip") == 0)
    {
        CA_myip();
    }
    else if (strcmp(argv[1], "myport") == 0)
    {
        CA_myport();
    }
    else if (strcmp(argv[1], "connect") == 0)
    {
        if(argc < 4)
        {
            printf("Missing the input! %d\n", argc);
        }
        else
        {
            CA_connect((char *)argv[2], PORT);
        }
    }
    else if (strcmp(argv[1], "list") == 0)
    {
        CA_list();
    }
    else if (strcmp(argv[1], "terminate") == 0)
    {
        CA_terminate();
    }
    else if (strcmp(argv[1], "send") == 0)
    {
        CA_send();
    }
    else if (strcmp(argv[1], "exit") == 0)
    {
        CA_exit();
    }
    else
    {
        printf("Error! Unknown command, please try again!");
        CA_help();
    }
    /* int port_no, len, opt;
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
    printf("Message received: %s\n", buffer); */
}

/*******************************************************************************************
 * Function name: CA_help
 * Functionality: Show the command usage
 * Requirement: CA-RS-FR-4
 ******************************************************************************************/
void CA_help()
{
    printf("\nUsage: CA [command] [ID] [Data] \n");
    printf("Command:\n");
    printf("\thelp:\t\t\t\tHiển thị hướng dẫn sử dụng các lệnh.\n");
    printf("\tmyip:\t\t\t\tHiển thị địa chỉ IP của máy đang chạy chương trình.\n");
    printf("\tmyport:\t\t\t\tHiển thị port mà chương trình đang lắng nghe kết nối.\n");
    printf("\tconnect <destination> <port>:\tTạo kết nối TCP đến một peer khác qua IP và port được chỉ định.\n");
    printf("\tlist:\t\t\t\tHiển thị danh sách các kết nối hiện tại, bao gồm cả kết nối được tạo bởi chương trình và kết nối từ các peer khác.\n");
    printf("\tterminate <connection id>:\tNgắt kết nối với một peer dựa trên ID từ danh sách list.\n");
    printf("\tsend <connection id> <message>:\tGửi tin nhắn tới peer được chỉ định (ID từ danh sách list).\n");
    printf("\texit:\t\t\t\tĐóng tất cả các kết nối hiện tại và thoát chương trình.\n\n");
} /* End of function CA_help */

/*******************************************************************************************
 * Function name: CA_myip
 * Functionality: Show the IP of computer
 * Requirement: CA-RS-FR-5
 ******************************************************************************************/
void CA_myip()
{
    struct ifaddrs *ifaddr, *ifa_index;
    char ip[INET_ADDRSTRLEN]; //The variable to store the ip as string
    
    if (getifaddrs(&ifaddr) == -1)
    {
        perror("Can't get the list of interface");
        return;
    }
    
    printf("The list of IP address:\n");
    for (ifa_index = ifaddr; ifa_index != NULL; ifa_index = ifa_index->ifa_next)
    {
        if ((ifa_index->ifa_addr != NULL) && (ifa_index->ifa_addr->sa_family == AF_INET))
        {
            // Store the address from the interface
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa_index->ifa_addr;

            // Convert the network address structure into a character string.
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
            printf("My IP address of %s: %s\n", ifa_index->ifa_name, ip);
        }
        else
        {
            // Do nothing
        }
    }
    
    // Free the memory which allocated by getifaddrs
    freeifaddrs(ifaddr);
    printf("Done!\n\n");
} /* End of function CA_myip */

/*******************************************************************************************
 * Function name: CA_myport
 * Functionality: Show the PORT of application
 * Requirement: CA-RS-FR-6
 ******************************************************************************************/
void CA_myport()
{
    printf("My port is: %d\n", PORT);
}

/*******************************************************************************************
 * Function name: CA_connect
 * Parameter:
 *      des: destinate address
 *      port: destinate port
 * Functionality: Show the PORT of application
 * Requirement: CA-RS-FR-8, CA-RS-FR-10, CA-RS-FR-11, CA-RS-FR-12
 ******************************************************************************************/
void CA_connect(char *des, int port)
{
    printf("To be connected to: %s; Port = %d\n", des, PORT);
}

void CA_list()
{
    printf("List is: xxx\n");
}

void CA_terminate()
{
    printf("To be termindated: xxx\n");
}

void CA_send()
{
    printf("To be send: xxx\n");
}

void CA_exit()
{
    printf("To be exit: xxx\n");
}