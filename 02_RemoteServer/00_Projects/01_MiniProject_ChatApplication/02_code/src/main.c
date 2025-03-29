#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <pthread.h>
#include <poll.h>

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define RECEIVE_ACCEPT 0

#define COMMAND_BUFFER_SIZE 1024
#define IP_BUFFER_SIZE 15

#define MAX_MYIP_SIZE   5
#define MAX_CONNECT_IP_SIZE   50

#define MAX_RECEIVE_LEN 100
#define MAX_TRANSMIT_LEN 100

#define MYIP_FILE_NAME  "./bin/myip_data.txt"
#define PORT 8080

/********************************************************
*                    TYPEDEF SECTION                    *
********************************************************/
typedef struct
{
    int socket_fd;
    struct sockaddr_in address;
} Connection_t;

/********************************************************
*                 CONSTANCE SECTION                     *
********************************************************/
const char *COMMAND_HELP = "help";
const char *COMMAND_MYIP = "myip";
const char *COMMAND_MYPORT = "myport";
const char *COMMAND_CONNECT = "connect";
const char *COMMAND_LIST = "list";
const char *COMMAND_TERMINATE = "terminate";
const char *COMMAND_SEND = "send";
const char *COMMAND_EXIT = "exit";

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
void CA_help();
void CA_myip();
void CA_myport();
void CA_connect(char *des, int port);
void CA_list();
void CA_terminate(int id);
void CA_send(int id, char *message);
void CA_exit();
void CA_StartServer(int port);
void *CA_ConnectionHandling(void *arg);

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
char gpMyIPList[MAX_MYIP_SIZE][INET_ADDRSTRLEN];
Connection_t gsConnectedIPList[MAX_CONNECT_IP_SIZE];
int giMyIPCount;
int giConnectedIPCount;
int giMyPort = 0;

int giServer_fd;

int main(int argc, char *argv[])
{
    char lpCommand[COMMAND_BUFFER_SIZE] = {0};

    if (argc < 2)
    {
        printf("Please input the target port!\n");
        return EXIT_SUCCESS;
    }
    CA_StartServer(atoi(argv[1]));

    // Loop to for command interface
    while(1)
    {
        printf("CA comment > ");
        fgets(lpCommand, COMMAND_BUFFER_SIZE, stdin);
        if (strncmp(lpCommand, COMMAND_HELP, strlen(COMMAND_HELP)) == 0)
        {
            CA_help();
        }
        else if (strncmp(lpCommand, COMMAND_MYIP, strlen(COMMAND_MYIP)) == 0)
        {
            CA_myip();
        }
        else if (strncmp(lpCommand, COMMAND_MYPORT, strlen(COMMAND_MYPORT)) == 0)
        {
            CA_myport();
        }
        else if (strncmp(lpCommand, COMMAND_CONNECT, strlen(COMMAND_CONNECT)) == 0)
        {
            char lpIP[IP_BUFFER_SIZE] = {0};
            int liPort = 0;

            sscanf (lpCommand, "connect %s %d", lpIP, &liPort);
            CA_connect(lpIP, liPort);
        }
        else if (strncmp(lpCommand, COMMAND_LIST, strlen(COMMAND_LIST)) == 0)
        {
            CA_list();
        }
        else if (strncmp(lpCommand, COMMAND_TERMINATE, strlen(COMMAND_TERMINATE)) == 0)
        {
            int liID = 0;
            sscanf (lpCommand, "terminate %d", &liID);
            CA_terminate(liID);
        }
        else if (strncmp(lpCommand, COMMAND_SEND, strlen(COMMAND_SEND)) == 0)
        {
            char lpMessage[MAX_TRANSMIT_LEN] = {0};
            int liID = 0;

            sscanf (lpCommand, "send %d %s", &liID, lpMessage);
            CA_send(liID, lpMessage);
        }
        else if (strncmp(lpCommand, COMMAND_EXIT, strlen(COMMAND_EXIT)) == 0)
        {
            CA_exit();
        }
        else
        {
            //printf("Error! Unknown command, please try again!");
            //CA_help();
        }
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
    return 1;
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
    char ip[INET_ADDRSTRLEN]; // The variable to store the ip as string
    int liMyIP_fd; // file descriptor to store the self-address

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("Can't get the list of interface");
        return;
    }
    
    liMyIP_fd = open(MYIP_FILE_NAME, O_WRONLY | O_CREAT | O_TRUNC, 0777);
    if (-1 == liMyIP_fd)
    {
        perror("Error opening file!");
        freeifaddrs(ifaddr);
        return;
    }
    else
    {
        // Do nothing
    }

    // Lock file before write
    if (flock(liMyIP_fd, LOCK_EX) == -1)
    {
        perror("Can't get write lock!");
        close(liMyIP_fd);
        freeifaddrs(ifaddr);
        return;
    }
    else
    {
        // Do nothing
    }

    printf("The list of IP address:\n");
    giMyIPCount = 0;
    for (ifa_index = ifaddr; ifa_index != NULL; ifa_index = ifa_index->ifa_next)
    {
        if ((ifa_index->ifa_addr != NULL) && (ifa_index->ifa_addr->sa_family == AF_INET))
        {
            // Store the address from the interface
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa_index->ifa_addr;

            // Convert the network address structure into a character string.
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
            printf("\tMy IP address of %s: %s\n", ifa_index->ifa_name, ip);

            // Store the my ip but ignore loop address
            if (strcmp(ifa_index->ifa_name, "lo") != 0)
            {
                strlcpy(gpMyIPList[giMyIPCount], ip, INET_ADDRSTRLEN);
                giMyIPCount  = (giMyIPCount + 1) % MAX_MYIP_SIZE;
            }

            // Test
            // for (int i = 0; i < giMyIPCount; i++)
            // {
            //     printf("gpMyIPList[%d] = %s\n", i, gpMyIPList[i]);
            //     printf("giMyIPCount = %d\n", giMyIPCount);
            // }
            // Write the IP to file
            write(liMyIP_fd, ifa_index->ifa_name, sizeof(ifa_index->ifa_name));
            write(liMyIP_fd, ":", 1);
            write(liMyIP_fd, ip, INET_ADDRSTRLEN);
            write(liMyIP_fd, "\n", 1);
        }
        else
        {
            // Do nothing
        }
    }
    
    // Free the memory which allocated by getifaddrs, close the file
    freeifaddrs(ifaddr);
    close(liMyIP_fd);
    printf("Done!\n\n");
} /* End of function CA_myip */

/*******************************************************************************************
 * Function name: CA_myport
 * Functionality: Show the PORT of application
 * Requirement: CA-RS-FR-6
 ******************************************************************************************/
void CA_myport()
{
    printf("\nMy port is: %d\n", giMyPort);
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
    int liClient_fd;
    int opt;
    struct sockaddr_in serv_addr;

    // Zero out the structure
    memset(&serv_addr, 0, sizeof(serv_addr));

    // If the myip comment is not run before, run it!
    if (giMyIPCount <= 0)
    {
        CA_myip();
    }
    else
    {
        // Do nothing
    }

    // Check the ip with self-address
    // Req: CA-RS-FR-11
    for (int index = 0; index < giMyIPCount; index++)
    {
        if (strncmp(des, gpMyIPList[index], INET_ADDRSTRLEN) == 0)
        {
            printf("Can't create the self-connection!\n");
            return;
        }
        else
        {
            // Do nothing
        }
    }

    // Create the socket
    liClient_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(liClient_fd < 0)
    {
        perror("Socket creation failed");
        return;
    }
    else
    {
        // Do nothing
    }

    setsockopt(liClient_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    //Convert IP address from text to binary
    // Req: CA-RS-FR-10
    if (inet_pton(AF_INET, des, &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid address/ Address not supported!\n");
        close(liClient_fd);
        return;
    }
    else
    {
        // Do nothing
    }

    // Check the ip with connected address
    //Req: CA-RS-FR-12
    for (int index = 0; index < giConnectedIPCount; index++)
    {
        if (serv_addr.sin_addr.s_addr == gsConnectedIPList[index].address.sin_addr.s_addr)
        {
            printf("This address already connected!\n");
            close(liClient_fd);
            return;
        }
        else
        {
            // Do nothing
        }
    }

    printf("Connecting to: %s; Port = %d ...\n", des, port);
    // Start connect
    if (connect(liClient_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Connect failed\n");
    }
    else
    {
        printf("Connected to: %s; Port = %d\n", des, port);
        // Store the connected address
        if (giConnectedIPCount < MAX_CONNECT_IP_SIZE)
        {
            gsConnectedIPList[giConnectedIPCount].socket_fd = liClient_fd;
            gsConnectedIPList[giConnectedIPCount].address = serv_addr;
            giConnectedIPCount  = (giConnectedIPCount + 1) % MAX_CONNECT_IP_SIZE;
        }
        else
        {
            printf("The storage is archived the limit, cant't store more than address!\n");
        }


        //Test connection
       //write(liClient_fd, "Hello!", 7);
    }

    // close(liClient_fd);
    return;
} /* End of function CA_connect */

/*******************************************************************************************
 * Function name: CA_list
 * Functionality: List all the connection
 * Requirement: CA-RS-FR-14
 ******************************************************************************************/
void CA_list()
{
    printf("Listing the addresses:\n");

    for (int index = 0; index < giConnectedIPCount; index++)
    {
        printf("\tID = %d;Address = %s, port = %d\n", index, inet_ntoa(gsConnectedIPList[index].address.sin_addr),
            ntohs(gsConnectedIPList[index].address.sin_port));
    }
}

/*******************************************************************************************
 * Function name: CA_terminate
 * Parameter:
 *      id: id of connection within the stored list
 * Functionality: Termindate the existed connection.
 * Requirement: CA-RS-FR-16, CA-RS-FR-17
 ******************************************************************************************/
void CA_terminate(int id)
{
    //Req: CA-RS-FR-16
    if ((id < 0 )|| (id >= giConnectedIPCount) || (giConnectedIPCount == 0))
    {
        printf("Invalid connection ID!\n");
        return;
    }

    //Close the connection
    close(gsConnectedIPList[id].socket_fd);

    // Remove from the list
    for (int index = id; index < giConnectedIPCount - 1; index++)
    {
        gsConnectedIPList[index] = gsConnectedIPList[index + 1];
    }
    giConnectedIPCount--;

    printf("Connection ID=%d termindated ID!\n", id);
}

/*******************************************************************************************
 * Function name: CA_send
 * Parameter:
 *      id: id of connection within the stored list
 *      message: The message intended to send, max 100 characters
 * Functionality: Send a message to intended connect which existed in storage.
 * Requirement: CA-RS-FR-19, CA-RS-FR-20
 ******************************************************************************************/
void CA_send(int id, char *message)
{
    if (id <0 || id >= giConnectedIPCount)
    {
        printf("Invalid connection ID!\n");
        return;
    }
    
    // if (connect(gsConnectedIPList[id].socket_fd, (struct sockaddr *)&gsConnectedIPList[id].address,
    //     sizeof(gsConnectedIPList[id].address)) == -1)
    // {
    //     printf("Connect failed\n");
    // }
        
    write(gsConnectedIPList[id].socket_fd, message, strlen(message) + 1);

    printf("Message \"%s\" sent to peer %d!\n", message, id);
}

/*******************************************************************************************
 * Function name: CA_exit
 * Functionality: Termindate all connection and exit program
 * Requirement: CA-RS-FR-26, CA-RS-FR-27 (T.B.D)
 ******************************************************************************************/
void CA_exit()
{
    //Req: CA-RS-FR-26
    for (int index; index < giConnectedIPCount; index++)
    {
        close(gsConnectedIPList[index].socket_fd);
    }
    //Close the server
    close(giServer_fd);

    printf("Exiting program...\n");
    // Delay to enhance user feel that the programm need to process something before exit :D
    sleep(1);
    exit(EXIT_SUCCESS);
}

/*******************************************************************************************
 * Function name: CA_StartServer
 * Functionality: Start Server to listen the peer connect
 ******************************************************************************************/
void CA_StartServer(int port)
{
    int len, opt = 1;
    int new_socket_fd;
    struct sockaddr_in serv_addr;

    //Store the port number
    giMyPort = port;

    giServer_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (giServer_fd < 0)
    {
        perror("Socket creation failed!");
        close(giServer_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        // Do nothing
    }

    if (setsockopt(giServer_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt(SO_REUSEADDR) failed");
        close(giServer_fd);
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(giMyPort);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(giServer_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("Bind failed. Error");
        close(giServer_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(giServer_fd, MAX_CONNECT_IP_SIZE) == -1)
    {
        printf("Listen failed\n");
        close(giServer_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Listening on port %d...\n",giMyPort);
    }
        
    pthread_t lThread_tid;
    pthread_create(&lThread_tid, NULL, CA_ConnectionHandling, NULL);

    // new_socket_fd = accept(giServer_fd, (struct sockaddr *)&client_addr, (socklen_t *)&len);

    // if (new_socket_fd == -1)
    //     printf("accept failed\n");
    // printf("client_addr.sin_addr.s_addr = %d\n",client_addr.sin_addr.s_addr);
    // read(new_socket_fd, buffer, sizeof(buffer));
    // printf("Message received: %s\n", buffer);
}

/*******************************************************************************************
 * Function name: CA_ConnectionHandling
 * Functionality: Thread to handle the receive message as well as store the connection
 * Requirement: CA-RS-FR-22, CA-RS-FR-23, CA-RS-FR-24
 ******************************************************************************************/
void *CA_ConnectionHandling(void *arg)
{
    struct sockaddr_in client_addr;
    socklen_t lsAddr_size = sizeof(client_addr);
    int liNewClientSocket_fd;
    char received_buffer[MAX_RECEIVE_LEN] = {0};
    int index = 1;

    #if RECEIVE_ACCEPT == 0 
    struct pollfd pfd;
    pfd.fd = giServer_fd;
    pfd.events = POLLIN;
    #endif

    // Runtime thread, loop for always execute
    while(1)
    {
        #if RECEIVE_ACCEPT == 1
        // Origin way to receive the message, but can't check the terminated connection
        liNewClientSocket_fd = accept(giServer_fd, (struct sockaddr *)&client_addr, (socklen_t *)&lsAddr_size);
        if (-1 == liNewClientSocket_fd)
        {
            perror("Accept failed!");
            continue;
        }

        read(liNewClientSocket_fd, received_buffer, sizeof(received_buffer));
        printf("\nReceived message:\n");
        printf("\t%s:%d> %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), received_buffer);

        // Store the accepted address
        for (index = 0; index <= giConnectedIPCount; index++)
        {
            if (client_addr.sin_addr.s_addr == gsConnectedIPList[index].address.sin_addr.s_addr)
            {
                // printf("sender address already stored!\n");
                break;
            }
            else
            {
                // Do nothing
            }
        }
        //printf("index = %d\n", index);
        if (giConnectedIPCount < MAX_CONNECT_IP_SIZE)
        {
            // If the new connect is not existed in history, store new one
            if(index > giConnectedIPCount)
            {
                gsConnectedIPList[giConnectedIPCount].socket_fd = liNewClientSocket_fd;
                gsConnectedIPList[giConnectedIPCount].address = client_addr;
                giConnectedIPCount  = (giConnectedIPCount + 1) % MAX_CONNECT_IP_SIZE;
            }
        }
        else
        {
            printf("The storage is archived the limit, cant't store more than address!\n");
            close(liNewClientSocket_fd);
        }
        #else
        int ret = poll(&pfd, 1, 5000); // Timeout = 5000ms (5 sec)
        if (ret > 0 && (pfd.revents & POLLIN))
        {
            liNewClientSocket_fd = accept(giServer_fd, (struct sockaddr *)&client_addr, (socklen_t *)&lsAddr_size);
            if (liNewClientSocket_fd > 0)
            {
                printf("Client connected!\n");
                char client_ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
                printf("Client connected from %s:%d\n", client_ip, ntohs(client_addr.sin_port));
                while (1)
                {
                    memset(received_buffer, 0, MAX_RECEIVE_LEN);
                    int bytes_received = read(liNewClientSocket_fd, received_buffer, sizeof(received_buffer) - 1);
                    if (bytes_received == 0)
                    {
                        printf("Client disconnected\n");
                        break;
                    }
                    else if (bytes_received < 0)
                    {
                        perror("Read error");
                        break;
                    }
                    else
                    {
                        received_buffer[bytes_received] = '\0';
                        printf("Received: %s\n", received_buffer);
                    }
                }
            }
        } 
        else 
        {
            printf("No incoming connections (timeout reached)\n");
        }
        #endif
    }
}