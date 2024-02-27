// Server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <signal.h>


#include "constants.h"
#include "ip.h"
#include "utils.h"

// #define MAX_PACKET_SIZE 1001 // 1000 bytes for data + 1 bytes for sequence number

#define DEFAULT_PORT 44444
#define MAXIMUM_BINDS 10
#define WAIT_TIME 555000

// global variables:
int server_socket;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Signal handler for SIGIO
void sigpoll_handler(int signum) {
    char buffer[1];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    
    // Receive data packet
    ssize_t recv_size = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*) &client_addr, &addr_len);
    if (recv_size < 0) {
        perror("recvfrom failed");
        return;
    }
    
    // Process the received packet
    printf("%s\n", buffer);
    // Process the data in 'buffer'
}



int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1; // Return an error code
    }
    // int server_socket;
    int server_port = atoi(argv[2]); //DEFAULT_PORT;
    struct sockaddr_in server_address, client_address;
    socklen_t addr_len = sizeof(client_address);
    char server_ip[16];

    struct sigaction sa;

    // Create a UDP socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Socket creation error");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr(argv[1]); //htonl(INADDR_ANY);

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
        char *ipStr = convertSockaddrToIPString((struct sockaddr *)&client_address);
        printf("recieved initial request from %s with %d bytes\n", ipStr, bytes_received);

        // only open the file if it is exists
        char directory[] = "tmp/server/";
        char full_path[strlen(directory) + strlen(buffer) + 1];
        // Copy directory to the combined string
        strcpy(full_path, directory);
        // Concatenate filename to the combined string
        strcat(full_path, buffer);
        // char *full_file_dir = "";
        // sprintf(full_file_dir, "tmp/server/%s", buffer);

        if (access(full_path, F_OK) != -1) {
        // get full file directory
            printf("Sending file %s\n", full_path);
            FILE *file = fopen(full_path, "rb");
            if (file == NULL) {
                error("Error opening file");
            }

            // get file size
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            printf("file size; %lu\n", file_size);



        /////////////////////////////////
                    // Set the socket to non-blocking mode
            fcntl(server_socket, F_SETFL, O_NONBLOCK);

            // Set up signal handler for SIGPOLL
            memset(&sa, 0, sizeof(sa));
            sa.sa_handler = &sigpoll_handler;
            if (sigaction(SIGPOLL, &sa, NULL) == -1) {
                perror("sigaction failed");
                exit(EXIT_FAILURE);
            }

            // Enable asynchronous I/O on the socket
            if (fcntl(server_socket, F_SETFL, fcntl(server_socket, F_GETFL) | O_ASYNC) < 0) {
                perror("fcntl O_ASYNC failed");
                exit(EXIT_FAILURE);
            }
        /////////////////////////////////////


            // send file size to client
            unsigned char sizeBytes[3];
            sizeBytes[0] = (file_size >> 16) & 0xFF;
            sizeBytes[1] = (file_size >> 8) & 0xFF;
            sizeBytes[2] = file_size & 0xFF;


            if (sendto(server_socket, sizeBytes, sizeof(sizeBytes), 0, (struct sockaddr*)&client_address, addr_len) == -1) {
                perror("Sendto error");
            }

            // read file
            char *file_buffer = malloc(file_size);
            if (file_buffer == NULL) {
                error("Memory allocation failed");
            }

            fread(file_buffer, 1, file_size, file);
            fclose(file);


            int bytes_sent;
            int remaining_bytes = file_size;
            char *ptr = file_buffer;

            // send file packets
            while (remaining_bytes > 0) {
                int bytes_to_send = remaining_bytes > MAX_PACKET_SIZE-1 ? MAX_PACKET_SIZE-1 : remaining_bytes;
                memcpy(buffer + 1, ptr, bytes_to_send);
                // memcpy(buffer, &seq_num, 1);
                // TODO send packets with bad order and test.
                // if (seq_num != 5){
                //     seq_num = 6;
                memcpy(buffer, &seq_num, 1);
                printf("sending %d bytes with sequence number %d \n", bytes_to_send, seq_num);
                bytes_sent = sendto(server_socket, buffer, bytes_to_send + 1, 0, (struct sockaddr *)&client_address, addr_len);
                if (bytes_sent < 0) {
                    error("Error sending packet");
                }
                usleep(10000);
                ptr += bytes_to_send;
                remaining_bytes -= bytes_to_send;
                seq_num++;

            }

        
        }
    }

    close(server_socket);
    return 0;
}
