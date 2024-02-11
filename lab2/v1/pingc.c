#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "ip.h"
#include "constants.h"


#define MAX_ATTEMPTS 10
#define TIMEOUT 1.0
#define INITIAL_SEQ 0
#define CONTROL 1

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s server_ip server_port\n", argv[0]);
        return 1; // Return an error code
    }
    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);

    int client_socket;
    struct sockaddr_in server_address;
    struct timeval start_time, end_time;
    socklen_t addr_len = sizeof(server_address);

    // Create a UDP socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_socket == -1) {
        perror("Socket creation error");
        exit(1);
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);

    char client_ip[16];
    get_ip(client_ip);
    printf("pingc %s %d %s %%\n", server_ip, server_port, client_ip);

    // initial sequence number is zero
    int sequence_number = INITIAL_SEQ;
    uint16_t sequence_number_network; // Network byte order
    uint8_t control_message;
    char payload[97]; // 100 bytes total (2 bytes seq, 1 byte control, 97 bytes payload)
    
    char message[BUFFER_SIZE];
    
    while (sequence_number < MAX_ATTEMPTS) {
        sequence_number++;
        printf("pingc %s %d %s %% ", server_ip, server_port, client_ip);

        // snprintf(message, sizeof(message), "PING %d", sequence_number);

        gettimeofday(&start_time, NULL);

        // Populate the message components
        sequence_number_network = sequence_number; 
        control_message = CONTROL;  // Change this value as needed
        memset(payload, 'A', sizeof(payload)); // Fill the payload with 'A' characters


        // Construct the message buffer
        memcpy(message, &sequence_number_network, sizeof(sequence_number_network));
        memcpy(message + sizeof(sequence_number_network), &control_message, sizeof(control_message));
        memcpy(message + sizeof(sequence_number_network) + sizeof(control_message), payload, sizeof(payload));


        // Send ping message to the server
        if (sendto(client_socket, message, sizeof(message), 0, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
            perror("Sendto error");
            exit(1);
        }


        // Set a timeout for receiving the response
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT;
        timeout.tv_usec = 0;
        if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
            perror("Setsockopt error");
            exit(1);
        }

        char response[BUFFER_SIZE];
        memset(response, 0, sizeof(response));

        // Receive the response from the server
        ssize_t bytes_received = recvfrom(client_socket, response, sizeof(response), 0, (struct sockaddr*)&server_address, &addr_len);
        if (bytes_received == -1) {
            printf("Ping request %d timed out\n", sequence_number);
        } else {
            uint16_t recieved_seq_number = *(uint16_t*)(response);
            if (recieved_seq_number == sequence_number){
                gettimeofday(&end_time, NULL);
                double rtt = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec) / 1000.0;
                printf("Response from %s: Seq=%u, Control=%u, RTT: %.2f ms\n", server_ip,
                   recieved_seq_number, response[2], rtt);
            } else {
                printf("Sequence number not matching on response from %s : expected %u recieved %u", server_ip, sequence_number, recieved_seq_number);
            }
        }
    }

    close(client_socket);
    return 0;
}
