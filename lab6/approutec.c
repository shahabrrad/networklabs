#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int main() {
    char router_ip[INET_ADDRSTRLEN];
    int tcp_port, next_hop_port;
    char next_hop_ip[INET_ADDRSTRLEN];

    while (1) {
        printf("? ");
        fflush(stdout);

        // Read user input
        if (scanf("%s %d %s %d", router_ip, &tcp_port, next_hop_ip, &next_hop_port) != 4) {
            fprintf(stderr, "Invalid input\n");
            exit(EXIT_FAILURE);
        }

        // Create TCP socket
        int tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_socket == -1) {
            perror("TCP socket creation failed");
            exit(EXIT_FAILURE);
        }

        // Connect to router-ip tcp-port
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(tcp_port);
        if (inet_pton(AF_INET, router_ip, &server_addr.sin_addr) <= 0) {
            perror("Invalid address/ Address not supported");
            exit(EXIT_FAILURE);
        }

        if (connect(tcp_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
            perror("TCP connection failed");
            close(tcp_socket);
            continue; // Continue to next iteration
        }

        // Send final destination's IPv4 address to the server
        struct in_addr final_dest_addr;
        if (inet_pton(AF_INET, next_hop_ip, &final_dest_addr) <= 0) {
            perror("inet_pton");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }

        if (send(tcp_socket, &final_dest_addr.s_addr, sizeof(uint32_t), 0) == -1) {
            perror("send");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }

        // Send final destination's port number to the server
        uint16_t final_dest_port_n = htons(next_hop_port);
        if (send(tcp_socket, &final_dest_port_n, sizeof(uint16_t), 0) == -1) {
            perror("send");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }


        // Close socket
        close(tcp_socket);
    }

    return 0;
}
