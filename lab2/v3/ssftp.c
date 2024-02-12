// Client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

#include "constants.h" 

#define MAX_PACKET_SIZE 1002 // 1000 bytes for data + 2 bytes for sequence number
#define MAX_PACKETS 8      // Maximum number of packets to store in memory

#define TIMEOUT 500000

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void alarm_handler(int signum) {
    fprintf(stderr, "Timeout: No response from the server.\n");
    exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {
    FILE *file;
    struct sigaction sa;
    if (argc != 6) {
        printf("Usage: %s <file_name> <server_ip> <server_port> <client_ip> <packet_size>\n", argv[0]);
        return 1; // Return an error code
    }
    char *filename = argv[1];
    char *server_ip = argv[2];
    int server_port = atoi(argv[3]);
    // char *client_ip = argv[4];
    // int packet_size = atoi(argv[5]);

    char file_name[FILENAME_LENGTH + 1]; // + 1];
    strcpy(file_name, filename);


    // Pad filename if it's shorter than FILENAME_LENGTH
    int len = strlen(file_name);
    if (len < FILENAME_LENGTH) {
        memset(file_name + len, 'Z', FILENAME_LENGTH - len);
        // file_name[FILENAME_LENGTH] = '\0'; // Ensure null termination
    }
    printf("%s\n", file_name);
    // return 0;

    int client_socket;
    struct sockaddr_in server_address;
    // struct timeval start_time, end_time;
    // socklen_t addr_len = sizeof(server_address);

    // Create a UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Socket creation error");
        exit(1);
    }
    printf("%s, %d \n", server_ip, server_port);

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);



    char message[MAX_PACKET_SIZE];
    char initial_message[FILENAME_LENGTH];
    
    memcpy(initial_message, &file_name, 10); //sizeof(file_name));

    int seq_num = 0;
    int bytes_received;
    int last_seq_num = -1;
    char *packets[MAX_PACKETS];
    int packets_received = 0;

    // Set up signal handler for SIGALRM
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    // Set alarm
    ualarm(TIMEOUT, 0); // Set alarm for 1 second
    // usleep(500000);
    if (sendto(client_socket, initial_message, sizeof(initial_message), 0, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
            perror("Sendto error");
            exit(1);
        }
    
    file = fopen(filename, "wb");
    if (file == NULL) {
        error("Error opening file");
    }

    while (1) {
        bytes_received = recvfrom(client_socket, message, MAX_PACKET_SIZE, 0, NULL, NULL);
        if (bytes_received <= 0) {
            break;
        }
        ualarm(0, 0);

        // TODO wait for the file size

        int received_seq_num;
        memcpy(&received_seq_num, message, 2);

        printf("recieved %d bytes with sequence number %d expected %d\n", bytes_received, received_seq_num, seq_num);

        if (received_seq_num != seq_num) {
            fprintf(stderr, "Packet out of order\n");
            continue;
        }

        // TODO change this so that the last packet recieved is saved in smaller size
        if (received_seq_num > last_seq_num) {
            packets[packets_received] = malloc(bytes_received - 2);
            if (packets[packets_received] == NULL) {
                error("Memory allocation failed");
            }
            memcpy(packets[packets_received], message + 2, bytes_received - 2);
            last_seq_num = received_seq_num;
            packets_received++;
        }

        // TODO: change it so that it can detect last packet if size is equal.
        // I should calculate number of packets based on file size
        if (bytes_received < MAX_PACKET_SIZE) { // Last packet received
            break;
        }

        seq_num++;
    }

    // Write all packets into the file
    // TODO handle lost packets
    for (int i = 0; i < packets_received; i++) {
        printf("writing packets %d\n", i);
        fwrite(packets[i], 1, MAX_PACKET_SIZE - 2, file);
        free(packets[i]);
    }

    fclose(file);

    close(client_socket);
    return 0;
}

