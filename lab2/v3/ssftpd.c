// Server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <sys/socket.h>

#include "constants.h"
#include "ip.h"
#include "utils.h"

#define MAX_PACKET_SIZE 1001 // 1000 bytes for data + 2 bytes for sequence number

#define DEFAULT_PORT 44444
#define MAXIMUM_BINDS 10
#define WAIT_TIME 555000

void error(const char *msg) {
    perror(msg);
    exit(1);
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
        int bytes_received;
        char buffer[MAX_PACKET_SIZE];
        memset(buffer, 0, sizeof(buffer));
        uint8_t seq_num = 0;

        // Receive a file_name
        // ssize_t bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &addr_len);
        bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &addr_len);
        remove_trailing_Z(buffer);
        printf("%s , %d\n", buffer, bytes_received);

        // only open the file if it is exists
if (access(buffer, F_OK) != -1) {
FILE *file = fopen(buffer, "rb");
    if (file == NULL) {
        error("Error opening file");
    }

        fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("file size; %lu\n", file_size);
    // return 1;
    // send file size to client
    unsigned char sizeBytes[3];
    sizeBytes[0] = (file_size >> 16) & 0xFF;
    sizeBytes[1] = (file_size >> 8) & 0xFF;
    sizeBytes[2] = file_size & 0xFF;
    // printf("buffer %s\n", *(uint16_t*)buffer);

    if (sendto(server_socket, sizeBytes, sizeof(sizeBytes), 0, (struct sockaddr*)&client_address, addr_len) == -1) {
                perror("Sendto error");
            }
    char *file_buffer = malloc(file_size);
    if (file_buffer == NULL) {
        error("Memory allocation failed");
    }

    fread(file_buffer, 1, file_size, file);
    fclose(file);


    int bytes_sent;
    int remaining_bytes = file_size;
    char *ptr = file_buffer;

    // ssize_t bytes_received = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cli_addr, clilen);

    while (remaining_bytes > 0) {
        int bytes_to_send = remaining_bytes > 1000 ? 1000 : remaining_bytes;
        memcpy(buffer + 1, ptr, bytes_to_send);
        memcpy(buffer, &seq_num, 1);
        // TODO send packets with bad order and test.
        // if (seq_num != 5){
        printf("sending %d bytes with sequence number %d \n", bytes_sent, seq_num);
        bytes_sent = sendto(server_socket, buffer, bytes_to_send + 1, 0, (struct sockaddr *)&client_address, addr_len);
        if (bytes_sent < 0) {
            error("Error sending packet");
        }
        // }
        ptr += bytes_to_send;
        remaining_bytes -= bytes_to_send;
        seq_num++;
    }
        
}
    }

    close(server_socket);
    return 0;
}