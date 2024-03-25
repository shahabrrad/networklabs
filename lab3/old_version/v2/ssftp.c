// Client

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/time.h>

#include "constants.h" 

// #define MAX_PACKET_SIZE 1001 // 1000 bytes for data + 1 bytes for sequence number

#define TIMEOUT 500000
#define TIMEOUT_USEC 100000

// global variables
unsigned short seqmax;
unsigned short seqmin;
char** packets = NULL;
int client_socket;
struct sockaddr_in server_address;
int number_of_packet;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void alarm_handler(int signum) {
    fprintf(stderr, "Timeout: No response from the server.\n");
    exit(EXIT_FAILURE);
}

void alarm_handler_lostpacket(int signum) {
    printf("inside handler\n");
    unsigned short missing_seq_num;
    for(missing_seq_num = seqmin; missing_seq_num < seqmax+1; missing_seq_num++) {
        // printf("missing seq no: %d \n", missing_seq_num);
        if (packets[missing_seq_num] == NULL && missing_seq_num < number_of_packet) {
            // printf("seq number not recieved: %d\n", missing_seq_num);
            char buf[2] = {missing_seq_num, '\0'};
            if(sendto(client_socket, buf, sizeof(buf), 0, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
                perror("Sendto error");
                exit(1);
            } //;
            printf("packet sent with seq no %d\n", missing_seq_num);
        }
    }
    if (seqmin < number_of_packet){
    ualarm(TIMEOUT_USEC, 0);
    }
    // fprintf(stderr, "Timeout: No response from the server.\n");
    // exit(EXIT_FAILURE);
}


int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <file_name> <server_ip> <server_port> <client_ip> <packet_size>\n", argv[0]);
        return 1; // Return an error code
    }
    char *filename = argv[1];
    char *server_ip = argv[2];
    int server_port = atoi(argv[3]);
    // char *client_ip = argv[4];
    // int packet_size = atoi(argv[5]);

    FILE *file;
    long filesize;
    struct timeval start_time, end_time;
    struct sigaction sa;
    // unsigned short seqmax;
    // unsigned short seqmin;
    // struct sigaction sa_lostpacket;
    // struct itimerval timer;



    char file_name[FILENAME_LENGTH + 1]; 
    strcpy(file_name, filename);


    // Pad filename if it's shorter than FILENAME_LENGTH
    int len = strlen(file_name);
    if (len < FILENAME_LENGTH) {
        memset(file_name + len, 'Z', FILENAME_LENGTH - len);
        // file_name[FILENAME_LENGTH] = '\0'; // Ensure null termination
    }




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



    unsigned char message[MAX_PACKET_SIZE];
    char initial_message[FILENAME_LENGTH];
    
    memcpy(initial_message, &file_name, 10); 

    int seq_num = 0;
    int bytes_received;

    // Set up signal handler for SIGALRM
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);


    ualarm(TIMEOUT, 0); // Set alarm for 1 second

    gettimeofday(&start_time, NULL);    // start recording time

    // send initial request: file name
    if (sendto(client_socket, initial_message, sizeof(initial_message), 0, (struct sockaddr*)&server_address, sizeof(server_address)) == -1) {
            perror("Sendto error");
            exit(1);
        }
    
    // Get file size in the first package
    bytes_received = recvfrom(client_socket, message, sizeof(message), 0, NULL, NULL);
    if (bytes_received != 3){
        error("File size not sent from server in first packet");
    }
    // disable alarm once the first package is recieved
    ualarm(0, 0);
    filesize = (message[0] << 16) | (message[1] << 8) | message[2];


    // calculate number of packets and last packet:
    number_of_packet = 0;
    int last_packet_size = (filesize % (MAX_PACKET_SIZE - 1)) + 1;
    printf("last packet size will be: %d \n", last_packet_size);
    if (last_packet_size != 1){
        number_of_packet = filesize / (MAX_PACKET_SIZE - 1) + 1;
    }else{
        number_of_packet = filesize / (MAX_PACKET_SIZE - 1) ;
    }
    printf("will recieve %d packets for file with size %lu \n", number_of_packet, filesize);

    int number_of_recievied_packets = 0;
    double number_of_bytes_recieved = 0; 

    // char *packets[number_of_packet];
    packets = (char**)malloc(number_of_packet * sizeof(char*));
    // initialise the array to save packets
    for (int i = 0; i < number_of_packet; i++) {
        packets[i] = NULL;
    }




    // Set up the signal handler for lost packets
    signal(SIGALRM, alarm_handler_lostpacket); // Register the handler for SIGALRM
    ualarm(TIMEOUT_USEC, 0); // Set the initial alarm for 1 second (1000 milliseconds)



    seqmax = 0;
    seqmin = 0;
    while (1) {
        bytes_received = recvfrom(client_socket, message, MAX_PACKET_SIZE, 0, NULL, NULL);
        if (bytes_received <= 0) {
            break;
        }
        number_of_recievied_packets++;
        number_of_bytes_recieved = number_of_bytes_recieved + (bytes_received - 1);
        // int received_seq_num;
        uint8_t received_seq_num;
        memcpy(&received_seq_num, message, 1);

        // if (received_seq_num == seqmin){
        //     seqmin++;
        // }
        // if (received_seq_num >= seqmax){
        //     seqmax = received_seq_num + 1;
        // }
        // for(int i = seqmin; i < seqmax+1; i++) {
        //      if (packets[i] != NULL) {
        //         seqmin = i;
        //      }else{
        //         break;
        //      }
        // }

        printf("recieved %d bytes with sequence number %d expected %d\n", bytes_received, received_seq_num, seq_num);

        // no need to check packet order
        // if (received_seq_num != seq_num) {
        //     fprintf(stderr, "Packet out of order\n");
        //     continue;
        // }

        packets[received_seq_num] = malloc(bytes_received - 1);
            if (packets[received_seq_num] == NULL) {
                error("Memory allocation failed");
            }
        memcpy(packets[received_seq_num], message + 1, bytes_received - 1);
        // printf("recieved %d, min %d , max %d \n", received_seq_num, seqmin, seqmax);


        if (received_seq_num >= seqmax){
            seqmax = received_seq_num + 1;
        }
        for(int i = seqmin; i < seqmax+1; i++) {
             if (packets[i] != NULL) {
                seqmin = i+1;
             }else{
                break;
             }
        }
        printf("recieved %d, min %d , max %d \n", received_seq_num, seqmin, seqmax);

        if (seqmin >= number_of_packet){    // last packet is recieved
            ualarm(0, 0);
            break;
            // ualarm(0, 0);
        }

        seq_num++;
    }

    // create the file
    char directory[] = "tmp/client/";
    char full_path[strlen(directory) + strlen(filename) + 1];
        // Copy directory to the combined string
    strcpy(full_path, directory);
        // Concatenate filename to the combined string
    strcat(full_path, filename);
    printf("%s \n", full_path);
    file = fopen(full_path, "wb");
    if (file == NULL) {
        error("Error opening file");
    }
    // write the packets
    int i;
    for (i = 0; i < number_of_packet; i++) {
        if (packets[i] == NULL){
            printf("null packet in %d \n", i);
        }else{
        printf("writing packets %d \n", i);
        if (i == number_of_packet-1){   // handle last packet
            if (last_packet_size != 1){
                fwrite(packets[i], 1, last_packet_size-1, file);    //if last packet size is different
                free(packets[i]);
            }else {
                fwrite(packets[i], 1, MAX_PACKET_SIZE-1, file);
            free(packets[i]);
            }
        }else{
        fwrite(packets[i], 1, MAX_PACKET_SIZE-1, file);
        free(packets[i]);
        }
        }

    }
    // write the last packets
    // printf("writing packets %d, %lu\n", i, sizeof(packets[i]));
    //     fwrite(packets[i], 1, last_packet_size-1, file);
    //     free(packets[i]);

    fclose(file);

    //  calculate the round trip time
    gettimeofday(&end_time, NULL);
    
    double rtt = (end_time.tv_sec - start_time.tv_sec) * 1000.0 + (end_time.tv_usec - start_time.tv_usec) / 1000.0;
    double percentage_recieved = 1-(number_of_bytes_recieved/filesize);
    printf("**** Process Finished ****\n");
    printf("Received %d packets out of %d possible packets\n", number_of_recievied_packets, number_of_packet);
    printf("Recieved %f bytes out of %lu possible bytes. loss percentage: %% %f \n", number_of_bytes_recieved, filesize, percentage_recieved);
    printf("Time to recieve all packets: %f ms \n Averge transfer time: %f bps\n", rtt , (number_of_bytes_recieved * 8)/(rtt / 1000));

    close(client_socket);
    return 0;
}

