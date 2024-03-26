#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "ip.h"
#include "constants.h"

#define DEFAULT_PORT 44444
#define MAXIMUM_BINDS 10
#define WAIT_TIME 555000


int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s server_ip server_port\n", argv[0]);
        return 1; // Return an error code
    }
    int server_socket;
    int server_port = atoi(argv[2]);
    struct sockaddr_in server_address, client_address;
    socklen_t addr_len = sizeof(client_address);
    char server_ip[16];

    // Create a UDP socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Socket creation error");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind the socket to the server address and port
    int bind_attempts = 1;
    printf("Binding attempt no. %i\n", bind_attempts);
    while ((bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) && bind_attempts <MAXIMUM_BINDS){
        bind_attempts = bind_attempts + 1;
        server_port = server_port + 1;
        server_address.sin_port = htons(server_port);
        printf("Binding attempt no. %i\n", bind_attempts);
    }
    if (bind_attempts >= MAXIMUM_BINDS) {  //  10th bind attempt failed
        perror("Bind error");
        exit(1);
    }

    //  store server's ip in server_ip
    get_ip(server_ip);
    
    printf("pings %s %d %%\n", server_ip, server_port);

    while (1) {
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, sizeof(buffer));

        // Receive a ping request from a client
        ssize_t bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &addr_len);


        if (bytes_received == -1) {
            perror("Recvfrom error");
            continue;
        }

        // Extract the components of the message
        uint16_t sequence_number = *(uint16_t*)(buffer); 
        uint8_t control_message = buffer[2]; 
        // uncommnet this if we need to use the payload
        // char* payload = buffer + 3;

        printf("ping message recieved from %s seq no: %d , control: %d\n", inet_ntoa(*(struct in_addr*)&client_address.sin_addr.s_addr), sequence_number, control_message);

        if (control_message == 0) {
            // Respond immediately with the same payload
            if (sendto(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, addr_len) == -1) {
                perror("Sendto error");
            }
        } else if (control_message == 1) {
            // Delay the response by 555 milliseconds
            usleep(WAIT_TIME);
            if (sendto(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, addr_len) == -1) {
                perror("Sendto error");
            }
        } else {
            // For control_message >= 2, ignore the packet and don't respond, print error
            char *ipStr = convertSockaddrToIPString((struct sockaddr *)&client_address);
            printf("Invalid command recieved from %s\n", ipStr);
        }
        

    }

    close(server_socket);
    return 0;
}
