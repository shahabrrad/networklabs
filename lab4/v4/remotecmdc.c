#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#define PORT 8080
#define SERVER_IP "fe80::a6bb:6dff:fe44:ddb8"

void sigpipe_handler(int signum) {
    printf("SIGPIPE error with FIFO.\n");
}

int main() {
    int client_socket;
    struct sockaddr_in6 server_address;
    char command[1024];

    // int fd;
    // char message[128];
    if ((client_socket = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server address structure
    server_address.sin6_family = AF_INET6;
    server_address.sin6_port = htons(PORT);
    server_address.sin6_scope_id = 0x20;

    char ip_address[INET6_ADDRSTRLEN+6]; // +5 for possible "%eth0"
    snprintf(ip_address, sizeof(ip_address), "%s%%eth0", SERVER_IP);
    // // server_address.sin6_addr = ip_address;

    // printf("%s\n", ip_address);
    // // Convert IPv6 address string to binary
    printf("%s\n", ip_address);
    if (inet_pton(AF_INET6, ip_address, &server_address.sin6_addr) <= 0)
        perror("Invalid IPv6 address");

    printf("Client is waiting to connect...\n");

    // // Convert IPv4 and IPv6 addresses from text to binary form
    // if (inet_pton(AF_INET6, SERVER_IP, &server_address.sin6_addr) <= 0) {
    //     perror("Invalid address/ Address not supported");
    //     exit(EXIT_FAILURE);
    // }

    // Connect to the server
    if (connect(client_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server. Type a command (type 'exit' to quit):\n");
    char response[1024];
    // while (1) {
        printf("?");
        fgets(command, sizeof(command), stdin); //  get the message to send to the server
        if ( (strlen(command) > 30) ){ 
            printf("command length exceeds maximum 30: %s\n", command);
            fflush(stdin);
        }else{
            // if (strncmp(command, "exit", 4) == 0) {     //  exit program
            //     break;
            // }
            ualarm(500000, 0);
            ssize_t bytesWritten = write(client_socket, command, strlen(command));

            if (bytesWritten == -1) {
                perror("write");
                printf("An error occured while trying to write to FIFO\n");
                // break; // Exit the loop and terminate the program on write error
            }

            // Wait for response from the server
            ssize_t num_bytes = read(client_socket, response, sizeof(response));

            // Cancel the alarm
            ualarm(0, 0);

            if (num_bytes < 0) {
            perror("read failed");
            exit(EXIT_FAILURE);
        } else if (num_bytes == 0) {
            printf("No response received from server.\n");
            // exit(EXIT_FAILURE);
        } else {
            printf("Server Response: %s\n", response);
        }
        }
    // }

    close(client_socket);
    return 0;
}
