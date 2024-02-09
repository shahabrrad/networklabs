#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>


#include "ip.h"

#define DEFAULT_PORT 444444

char* sockaddrToString(const struct sockaddr *addr) {
    char ipString[INET6_ADDRSTRLEN];
    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *ipv4 = (struct sockaddr_in *)addr;
        inet_ntop(AF_INET, &(ipv4->sin_addr), ipString, INET6_ADDRSTRLEN);
    } else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)addr;
        inet_ntop(AF_INET6, &(ipv6->sin6_addr), ipString, INET6_ADDRSTRLEN);
    } else {
        return NULL; // Invalid address family
    }
    return strdup(ipString); // Allocate memory and copy the string
}


int main() {
    int server_socket;
    int server_port = DEFAULT_PORT;
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
    while ((bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) && bind_attempts <10){
        bind_attempts = bind_attempts + 1;
        server_port = server_port + 1;
        server_address.sin_port = htons(server_port);
        printf("Binding attempt no. %i\n", bind_attempts);
    }
    if (bind_attempts >= 10) {
        perror("Bind error");
        exit(1);
    }
    get_ip(server_ip);
    // printf("pings %s %d %%", server_ip, server_port);
    // printf("Ping server is listening on port %d\n", server_port);
    printf("pings %s %d %%\n", server_ip, server_port);
    while (1) {
        char buffer[100];
        memset(buffer, 0, sizeof(buffer));

        // Receive a ping request from a client
        ssize_t bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &addr_len);


        if (bytes_received == -1) {
            perror("Recvfrom error");
            continue;
        }

        // printf("%d , %c, %c, %d, %d, %d, %d\n", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6]);
        // Extract the components of the message
        uint16_t sequence_number = *(uint16_t*)(buffer); // ntohs(*(uint16_t*)(buffer));
        uint8_t control_message = buffer[2]; //ntohs(buffer[2]);
        // char* payload = buffer + 3;

        printf("sequence no: %d , control message: %d\n", sequence_number, control_message);

        if (control_message == 0) {
            // Respond immediately with the same payload
            if (sendto(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, addr_len) == -1) {
                perror("Sendto error");
            }
        } else if (control_message == 1) {
            // Delay the response by 555 milliseconds
            usleep(555000);
            if (sendto(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, addr_len) == -1) {
                perror("Sendto error");
            }
        } else {
            char* ipStr = sockaddrToString((struct sockaddr *)&client_address);
            printf("Invalid command recieved from %s\n", ipStr);
        }
        // For control_message == 2, ignore the packet and don't respond

    }

    close(server_socket);
    return 0;
}
