#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define SECRET_KEY "mysecret" // place holder fo hardcoding

int main(int argc, char *argv[]) {
    if (argc != 7) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <secret_key> <udp_client_ip> <final_dest_ip> <final_dest_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    char *secret_key = argv[3];
    char *udp_client_ip = argv[4];
    char *final_dest_ip = argv[5];
    int final_dest_port = atoi(argv[6]);

    // Create a socket for communication with the tunneling server
    int sockfd;
    struct sockaddr_in server_addr;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    // Send connection request to the server
    char connection_request = 'c';
    if (send(sockfd, &connection_request, sizeof(char), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send the secret key to the server
    if (send(sockfd, secret_key, strlen(secret_key), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send final destination's IPv4 address to the server
    struct in_addr final_dest_addr;
    if (inet_pton(AF_INET, final_dest_ip, &final_dest_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, &final_dest_addr.s_addr, sizeof(uint32_t), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send final destination's port number to the server
    uint16_t final_dest_port_n = htons(final_dest_port);
    if (send(sockfd, &final_dest_port_n, sizeof(uint16_t), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // Send UDP client's IPv4 address to the server
    struct in_addr udp_client_addr;
    if (inet_pton(AF_INET, udp_client_ip, &udp_client_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (send(sockfd, &udp_client_addr.s_addr, sizeof(uint32_t), 0) == -1) {
        perror("send");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Data sent to server successfully.\n");

    uint16_t tunnel_port;
    int valread = read(sockfd, &tunnel_port, sizeof(uint16_t));
    if (valread < 0) {
        perror("read");
    }
    printf("UDP port recieved %d \n", ntohs(tunnel_port));


    close(sockfd);

    return 0;
}
