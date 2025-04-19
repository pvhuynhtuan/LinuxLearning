/********************************************************
*                    INCLUDE SECTION                    *
********************************************************/
// include frame work header
#include <sys/ioctl.h>
#include <ifaddrs.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/socket.h>

// Include user's header
#include "main.h"
#include "SG_MainProcess.h"
#include "MP_ConnectionManager.h"
#include "MP_DataSharing.h"

/********************************************************
*                     DEFINE SECTION                    *
********************************************************/
#define CM_LOG_NEW_CONNECTION       "A sensor node with <%d> has opened a new connection, Address = %s, Port = %d\n"
#define CM_LOG_CLOSED_CONNECTION    "The sensor node with <%d> has closed the connection, Address = %s, Port = %d\n"

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

/********************************************************
*          FUNCTION DECLARATION SECTION                 *
********************************************************/
void CM_StartServer(int port);
void *CM_ConnectionHandling(void *arg);
void *CM_ReadHandling(void *arg);

void CM_Myip();
void CM_Terminate(int id);

void CM_StoreConnection(int fd, struct sockaddr_in addr);
void CM_Close();

#if (CM_ENABLE_CONNECT_FUNCTION == 1)
void CM_Connect(char *des, int port);
#endif

#if (CM_ENABLE_SEND_FUNCTION == 1)
void CM_Send(int id, char *message);
#endif

/********************************************************
*         GLOBAL VARIABLE DECLARATION SECTION           *
********************************************************/
char gpMyIPList[CM_MAX_MYIP_SIZE][INET_ADDRSTRLEN];
Connection_t gsConnectedIPList[CM_MAX_CONNECT_IP_SIZE];
int giMyIPCount;
int giConnectedIPCount;
int giMyPort = 0;
int giServer_fd;

int volatile giCMRequesState;

/*******************************************************************************************
 * Function name: CM_MainThread
 * Functionality: The thread function of Conenction Manager
 * Requirement: 
 ******************************************************************************************/
void *CM_MainThread(void * argv)
{
    int *lpPort = (int *)(argv);

    // Start the condition to loop this thread
    giCMRequesState = 1;

    CM_Myip();
    CM_StartServer(*lpPort);
    while(giCMRequesState == 1)
    {
        #if (CM_LOG_PRINT_ENABLE == 1)
        printf(YELLOW "\nConnection Manager > Info: port = %d" RESET "\n", giMyPort);
        CM_List();
        #endif
        sleep(5);
    }
    CM_Close();
}

/*******************************************************************************************
 * Function name: CM_StartServer
 * Functionality: Start Server to listen the peer connect
 ******************************************************************************************/
void CM_StartServer(int port)
{
    int len, opt = 1;
    int new_socket_fd;
    struct sockaddr_in serv_addr;

    //Store the port number
    giMyPort = port;

    giServer_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (giServer_fd < 0)
    {
        perror("Connection Manager > Socket creation failed!");
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

    if (listen(giServer_fd, CM_MAX_CONNECT_IP_SIZE) == -1)
    {
        printf("Connection Manager > Listen failed\n");
        close(giServer_fd);
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Connection Manager > Listening on port %d...\n",giMyPort);
    }
        
    pthread_t lThread_tid;
    pthread_create(&lThread_tid, NULL, CM_ConnectionHandling, NULL);
} /* End of CM_StartServer function */

/*******************************************************************************************
 * Function name: CM_ConnectionHandling
 * Functionality: Thread to store the connection
 * Requirement: 
 ******************************************************************************************/
void *CM_ConnectionHandling(void *arg)
{
    struct sockaddr_in client_addr;
    socklen_t lsAddr_size = sizeof(client_addr);
    int liNewClientSocket_fd;
    Connection_t lsHandlingConnection;

    // Runtime thread, loop for always execute
    while(1)
    {
        // Origin way to receive the message, but can't check the terminated connection
        liNewClientSocket_fd = accept(giServer_fd, (struct sockaddr *)&client_addr, (socklen_t *)&lsAddr_size);
        if (-1 == liNewClientSocket_fd)
        {
            perror("Accept failed!");
            continue;
        }

        printf(YELLOW "\nConnection Manager > New client connected from %s:%d\n" RESET, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        // Store the accepted address
        CM_StoreConnection(liNewClientSocket_fd, client_addr);

        // Create the structure to pass through the thread
        lsHandlingConnection.socket_fd = liNewClientSocket_fd;
        lsHandlingConnection.address = client_addr;

        // Create a new thread for the client read
        pthread_t client_thread;
        pthread_create(&client_thread, NULL, CM_ReadHandling, &lsHandlingConnection);
        pthread_detach(client_thread);  // Auto-clean up thread
    }
}

/*******************************************************************************************
 * Function name: CM_ReadHandling
 * Functionality: Thread to handle the receive message
 * Requirement: 
 ******************************************************************************************/
void *CM_ReadHandling(void *arg)
{
    Connection_t *lsConnectionRead = malloc(sizeof(Connection_t));
    *lsConnectionRead = *(Connection_t *)arg;
    char received_buffer[CM_MAX_RECEIVE_LEN] = {0};

    printf("\nConnection Manager > Reading from address: %s ...\n", inet_ntoa(lsConnectionRead->address.sin_addr));
    //Read the received data
    while(1)
    {
        // Zero out the structure
        memset(received_buffer, 0, sizeof(received_buffer));
        int bytes_received = read(lsConnectionRead->socket_fd, received_buffer, sizeof(received_buffer));
        if (bytes_received <= 0)
        {
            // The peer is disconnected, to be remove the stored ID
            //Req: 
            int liRemoveID = -1;
            for (int index = 0; index < giConnectedIPCount; index++)
            {
                // found the peer id
                if ((lsConnectionRead->address.sin_addr.s_addr == gsConnectedIPList[index].address.sin_addr.s_addr)
                    && (lsConnectionRead->address.sin_port == gsConnectedIPList[index].address.sin_port))
                {
                    liRemoveID = index;
                }
            }

            if (-1 != liRemoveID)
            {
                //Close the connection
                printf(RED "\nConnection Manager > The peer ID = %d, address = %s is disconnected, removed from list!" RESET "\n",
                    liRemoveID, inet_ntoa(gsConnectedIPList[liRemoveID].address.sin_addr));
                
                /*************** Send the log *******************/
                #if (CM_LOG_WRITER_ENABLE == 1)
                // Create the buffer
                char lpFifoBuffer[SG_MAX_LOG_LENGTH];
                memset(lpFifoBuffer, 0, SG_MAX_LOG_LENGTH);
                
                // Set the message
                sprintf(lpFifoBuffer, CM_LOG_CLOSED_CONNECTION,
                    liRemoveID, inet_ntoa(gsConnectedIPList[liRemoveID].address.sin_addr),
                    ntohs(gsConnectedIPList[liRemoveID].address.sin_port)
                );

                write(giWriteFifoFD, lpFifoBuffer, strlen(lpFifoBuffer) + 1);
                #endif /* End of #if (CM_LOG_WRITER_ENABLE == 1) */

                CM_Terminate(liRemoveID);
            }
            else
            {
                printf(RED "\nConnection Manager > Unknown peer disconnected! address = %s, return = %d" RESET "\n",
                    inet_ntoa(lsConnectionRead->address.sin_addr), bytes_received);
            }

            free(lsConnectionRead);
            pthread_exit(NULL);
        }
        else
        {
            printf(GREEN "\nConnection Manager > Message from %s:%d > %s" RESET "\n", 
                inet_ntoa(lsConnectionRead->address.sin_addr), ntohs(lsConnectionRead->address.sin_port), received_buffer);
            
            char *lpEndptr;
            double ldTemp = strtod(received_buffer, &lpEndptr); 
            if (received_buffer == lpEndptr)
            {
                printf("Connection Manager > Invalid data received!\n");
            }
            else
            {
                // Get the node ID
                int liReceivedID = -1;
                for (int index = 0; index < giConnectedIPCount; index++)
                {
                    // found the peer id
                    if ((lsConnectionRead->address.sin_addr.s_addr == gsConnectedIPList[index].address.sin_addr.s_addr)
                        && (lsConnectionRead->address.sin_port == gsConnectedIPList[index].address.sin_port))
                    {
                        liReceivedID = index;
                    }
                }

                // Print the temperature
                #if (CM_LOG_PRINT_ENABLE == 1)
                printf("Connection Manager > Temperature: %f\n", ldTemp);
                #endif

                // Send data
                SensorData_t lsSensorData;
                lsSensorData.address = lsConnectionRead->address;
                lsSensorData.SensorNodeID = liReceivedID;
                lsSensorData.Temperature = ldTemp;

                // Send data to Data Manager
                #if (CM_SEND_DATASHARE_TO_DM_ENABLE == 1)
                int liReturnValueDM = DS_QueuePush(lsSensorData, DS_DATA_MANAGER_PRIO);
                #if (CM_LOG_PRINT_ENABLE == 1)
                if (-1 == liReturnValueDM)
                {
                    printf("Connection Manager > Push the sensor data FAILED!\n");
                    perror("Failed");
                }
                else
                {
                    printf("Connection Manager > Push the sensor data SUCCESS!\n");

                    // Test read back
                    SensorData_t lsReadbackDataDM;
                    DS_QueueGet(&lsReadbackDataDM, DS_DATA_MANAGER_PRIO);
                    printf("Connection Manager > Read back data = %f\n\n", lsReadbackDataDM.Temperature);
                }
                #endif /* End of #if (CM_LOG_PRINT_ENABLE == 1) */
                #endif /* End of #if (CM_SEND_DATASHARE_TO_DM_ENABLE == 1) */

                // Send data to Storage Manager
                #if (CM_SEND_DATASHARE_TO_SM_ENABLE == 1)
                int liReturnValueSM = DS_QueuePush(lsSensorData, DS_STORAGE_MANAGER_PRIO);
                #if (CM_LOG_PRINT_ENABLE == 1)
                if (-1 == liReturnValueSM)
                {
                    printf("Connection Manager > Push the sensor data FAILED!\n");
                    perror("Failed");
                }
                else
                {
                    printf("Connection Manager > Push the sensor data SUCCESS!\n");

                    // Test read back
                    SensorData_t lsReadbackDataSM;
                    DS_QueueGet(&lsReadbackDataSM, DS_STORAGE_MANAGER_PRIO);
                    printf("Connection Manager > Read back data = %f\n\n", lsReadbackDataSM.Temperature);
                }
                #endif /* End of #if (CM_LOG_PRINT_ENABLE == 1) */
                #endif /* End of #if (CM_SEND_DATASHARE_TO_SM_ENABLE == 1) */
            }
        }
    }
} /* End of function CM_ReadHandling */

/*******************************************************************************************
 * Function name: CM_Myip
 * Functionality: Show the IP of computer
 * Requirement: 
 ******************************************************************************************/
void CM_Myip()
{
    struct ifaddrs *ifaddr, *ifa_index;
    char ip[INET_ADDRSTRLEN]; // The variable to store the ip as string

    if (getifaddrs(&ifaddr) == -1)
    {
        perror("Can't get the list of interface");
        return;
    }

    printf("Connection Manager > The list of IP address:\n");
    giMyIPCount = 0;
    for (ifa_index = ifaddr; ifa_index != NULL; ifa_index = ifa_index->ifa_next)
    {
        if ((ifa_index->ifa_addr != NULL) && (ifa_index->ifa_addr->sa_family == AF_INET))
        {
            // Store the address from the interface
            struct sockaddr_in *addr = (struct sockaddr_in *)ifa_index->ifa_addr;

            // Convert the network address structure into a character string.
            inet_ntop(AF_INET, &addr->sin_addr, ip, INET_ADDRSTRLEN);
            printf("\tConnection Manager > My IP address of %s: %s\n", ifa_index->ifa_name, ip);

            // Store the my ip but ignore loop address
            if (strcmp(ifa_index->ifa_name, "lo") != 0)
            {
                strlcpy(gpMyIPList[giMyIPCount], ip, INET_ADDRSTRLEN);
                giMyIPCount  = (giMyIPCount + 1) % CM_MAX_MYIP_SIZE;
            }
        }
        else
        {
            // Do nothing
        }
    }
    
    // Free the memory which allocated by getifaddrs, close the file
    freeifaddrs(ifaddr);
} /* End of function CM_Myip */

/*******************************************************************************************
 * Function name: CM_List
 * Functionality: List all the connection
 * Requirement: 
 ******************************************************************************************/
void CM_List()
{
    printf("Connection Manager > Total connecting node: %d\n", giConnectedIPCount);
    for (int index = 0; index < giConnectedIPCount; index++)
    {
        printf("\tID = %d; Address = %s, port = %d\n", index, inet_ntoa(gsConnectedIPList[index].address.sin_addr),
            ntohs(gsConnectedIPList[index].address.sin_port));
    }
    printf("\n");
} /* End of function CM_List */

/*******************************************************************************************
 * Function name: CM_Terminate
 * Parameter:
 *      id: id of connection within the stored list
 * Functionality: Termindate the existed connection.
 * Requirement: 
 ******************************************************************************************/
void CM_Terminate(int id)
{
    //Req: CA-RS-FR-16
    if ((id < 0 )|| (id >= giConnectedIPCount) || (giConnectedIPCount == 0))
    {
        printf("Connection Manager > Invalid connection ID!\n");
        return;
    }

    //Close the connection
    shutdown(gsConnectedIPList[id].socket_fd, SHUT_RDWR);
    close(gsConnectedIPList[id].socket_fd);

    // Remove from the list
    for (int index = id; index < giConnectedIPCount - 1; index++)
    {
        gsConnectedIPList[index] = gsConnectedIPList[index + 1];
    }
    giConnectedIPCount--;

    printf("Connection Manager > Connection ID=%d termindated ID!\n", id);
} /* End of CM_Terminate function */

/*******************************************************************************************
 * Function name: CM_Close
 * Functionality: Termindate all connection
 * Requirement: 
 ******************************************************************************************/
void CM_Close()
{
    //Req: 
    for (int index; index < giConnectedIPCount; index++)
    {
        close(gsConnectedIPList[index].socket_fd);
    }
    //Close the server
    close(giServer_fd);
    printf("Connection Manager > Exited!\n");
} /* End of CM_Close function */

/*******************************************************************************************
 * Function name: CM_StoreConnection
 * Functionality: Store the fd and address
 ******************************************************************************************/
void CM_StoreConnection(int fd, struct sockaddr_in addr)
{
    int existing_index;
    
    existing_index = -1;
    for (int index = 0; index < giConnectedIPCount; index++)
    {
        if ((addr.sin_addr.s_addr == gsConnectedIPList[index].address.sin_addr.s_addr)
            && (addr.sin_port == gsConnectedIPList[index].address.sin_port))
        {
            existing_index = index;
            break;
        }
        else
        {
            // Do nothing
        }
    }
    
    if (-1 == existing_index)
    {
        // If the new connect is not existed in history, store new one
        if (giConnectedIPCount < CM_MAX_CONNECT_IP_SIZE)
        {
            /*************** Send the log *******************/
            #if (CM_LOG_WRITER_ENABLE == 1)
            // Create the buffer
            char lpFifoBuffer[SG_MAX_LOG_LENGTH];
            memset(lpFifoBuffer, 0, SG_MAX_LOG_LENGTH);
            
            // Set the message
            sprintf(lpFifoBuffer, CM_LOG_NEW_CONNECTION,
                giConnectedIPCount, inet_ntoa(addr.sin_addr),
                ntohs(addr.sin_port)
            );

            write(giWriteFifoFD, lpFifoBuffer, strlen(lpFifoBuffer) + 1);
            #endif /* End of #if (CM_LOG_WRITER_ENABLE == 1) */

            gsConnectedIPList[giConnectedIPCount].socket_fd = fd;
            gsConnectedIPList[giConnectedIPCount].address = addr;
            giConnectedIPCount  = (giConnectedIPCount + 1) % CM_MAX_CONNECT_IP_SIZE;
        }
        else
        {
            printf("Connection Manager > The storage is archived the limit, cant't store more than address!\n");
        }
    }
    else
    {
        // Do nothing
    }
    
} /* End of CM_StoreConnection function */

#if (CM_ENABLE_CONNECT_FUNCTION == 1)
/*******************************************************************************************
 * Function name: CM_Connect
 * Parameter:
 *      des: destinate address
 *      port: destinate port
 * Functionality: Show the PORT of application
 * Requirement: 
 ******************************************************************************************/
void CM_Connect(char *des, int port)
{
    int liClient_fd;
    int opt;
    struct sockaddr_in serv_addr;

    // Zero out the structure
    memset(&serv_addr, 0, sizeof(serv_addr));

    // If the myip comment is not run before, run it!
    if (giMyIPCount <= 0)
    {
        CM_Myip();
    }
    else
    {
        // Do nothing
    }

    // Check the ip with self-address
    for (int index = 0; index < giMyIPCount; index++)
    {
        if (strncmp(des, gpMyIPList[index], INET_ADDRSTRLEN) == 0)
        {
            printf("Connection Manager > Can't create the self-connection!\n");
            return;
        }
        else
        {
            // Do nothing
        }
    }

    //Convert IP address from text to binary
    if (inet_pton(AF_INET, des, &serv_addr.sin_addr) <= 0)
    {
        printf("Connection Manager > Invalid address/ Address not supported!\n");
        return;
    }
    else
    {
        // Do nothing
    }

    // Check the ip with connected address
    for (int index = 0; index < giConnectedIPCount; index++)
    {
        if ((serv_addr.sin_addr.s_addr == gsConnectedIPList[index].address.sin_addr.s_addr)
            && (serv_addr.sin_port == gsConnectedIPList[index].address.sin_port))
        {
            printf("Connection Manager > This address already connected!\n");
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

    printf("Connection Manager > Connecting to: %s; Port = %d ...\n", des, port);
    // Start connect
    if (connect(liClient_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("Connection Manager > Connect failed\n");
        close(liClient_fd);
        return;
    }
    else
    {
        // Do nothing        
    }
    
    printf("Connection Manager > Connected to: %s; Port = %d\n", des, port);

    CM_StoreConnection(liClient_fd, serv_addr);

    // Start to read any data from this connect as well
    Connection_t *lsReadingConnection = malloc(sizeof(Connection_t));;

    // Create the structure to pass through the thread
    lsReadingConnection->socket_fd = liClient_fd;
    lsReadingConnection->address = serv_addr;

    printf("Connection Manager > Start to read from address: %s\n\n", inet_ntoa(lsReadingConnection->address.sin_addr));

    // Create a new thread for the client read
    pthread_t client_thread;
    pthread_create(&client_thread, NULL, CM_ReadHandling, (void *)lsReadingConnection);
    pthread_detach(client_thread);  // Auto-clean up thread
    
    return;
} /* End of function CM_Connect */
#endif

#if (CM_ENABLE_SEND_FUNCTION == 1)
/*******************************************************************************************
 * Function name: CM_Send
 * Parameter:
 *      id: id of connection within the stored list
 *      message: The message intended to send, max 100 characters
 * Functionality: Send a message to intended connect which existed in storage.
 * Requirement: 
 ******************************************************************************************/
void CM_Send(int id, char *message)
{
    int opt = 1;

    if (id <0 || id >= giConnectedIPCount)
    {
        printf("Connection Manager > Invalid connection ID!\n");
        return;
    }
    
    int bytes_send = write(gsConnectedIPList[id].socket_fd, message, strlen(message));

    printf("Connection Manager > %d bytes of message \"%s\" sent to peer %d!\n", bytes_send, message, id);
} /* End of CM_Send function */
#endif

/*******************************************************************************************
 * Function name: CM_Close
 * Functionality: Termindate all connection
 * Requirement: 
 ******************************************************************************************/
void CM_Exit()
{
    printf("Connection Manager > Exiting Connection Manager ...\n");
    giCMRequesState = 0;
} /* End of CM_Close function */