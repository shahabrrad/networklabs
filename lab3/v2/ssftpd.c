// Server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <signal.h>
#include <time.h>
#include <errno.h>


#include "constants.h"
#include "ip.h"
#include "utils.h"

// #define MAX_PACKET_SIZE 1001 // 1000 bytes for data + 1 bytes for sequence number
#define BUF_SIZE 1024
#define DEFAULT_PORT 44444
#define MAXIMUM_BINDS 10
#define WAIT_TIME 555000
#define TRANSFER_END_TIMEOUT 1.5 //1.5 seconds in miliseconds

// global variables:
int server_socket;
int transfer_mode;
long file_size;
char *file_buffer;
struct sockaddr_in server_address, client_address;
timer_t timerid;


void io_handler(int);

void sigalrm_handler(int);

void signal_handler(int sig, siginfo_t *si, void *uc) {
    // Handler code here
    // printf("Timer fired!\n");
    printf("Timer expired. No packets received for %.1f seconds.\n", 1.5);
    transfer_mode = 0;
}

// sets the sockets into blocking mode, not used
// void set_socket_blocking(int sockfd) {
//     int opts = fcntl(sockfd, F_GETFL);
//     if (opts < 0) {
//         perror("fcntl(F_GETFL)");
//         exit(EXIT_FAILURE);
//     }
    
//     opts &= ~O_NONBLOCK; // Clear the O_NONBLOCK flag
//     if (fcntl(sockfd, F_SETFL, opts) < 0) {
//         perror("fcntl(F_SETFL)");
//         exit(EXIT_FAILURE);
//     }
// }

// error handler
void error(const char *msg) {
    perror(msg);
    exit(1);
}



int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <server_port>\n", argv[0]);
        return 1; // Return an error code
    }
    // int server_socket;
    int server_port = atoi(argv[2]); //DEFAULT_PORT;
    // struct sockaddr_in server_address, client_address;
    socklen_t addr_len = sizeof(client_address);
    char server_ip[16];
    // struct sigaction sa;

    transfer_mode = 0;  //  flag showing if a file is being transfered

    // set random seed for random dropping packets
    // uncomment this when testing packet loss
    srand(time(NULL));

    // Create a UDP socket
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_socket == -1) {
        perror("Socket creation error");
        exit(1);
    }

    // Set server address informaiton
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
    
    printf("ssftp server: %s %d %%\n", server_ip, server_port);


    // save the original configuration of socket before setting up async IO
    int original_owner = fcntl(server_socket, F_GETOWN);
    int original_flags = fcntl(server_socket, F_GETFL);


    // stay alive for next client requests
    while (1) {
        transfer_mode = 0;  // file is not being transfered
        int bytes_received;
        char buffer[MAX_PACKET_SIZE];
        memset(buffer, 0, sizeof(buffer));
        uint8_t seq_num = 0;






    //     	signal(SIGIO,io_handler);




        // Receive a file_name
        // ssize_t bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &addr_len);
        bytes_received = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_address, &addr_len);
        if (bytes_received < 10){
            continue;
        }
        remove_trailing_Z(buffer);  // decode file name

        // print the request recieved
        char *ipStr = convertSockaddrToIPString((struct sockaddr *)&client_address);
        printf("recieved initial request from %s with %d bytes\n", ipStr, bytes_received);
        printf("buffer0 %d\n", buffer[0]);

        
        // get the complete file path from the file name
        char directory[] = "tmp/server/";
        char full_path[strlen(directory) + strlen(buffer) + 1];
        // Copy directory to the combined string
        strcpy(full_path, directory);
        // Concatenate filename to the combined string
        strcat(full_path, buffer);


        // only open the file if it is exists
        if (access(full_path, F_OK) != -1) {
            transfer_mode = 1;  // file is being transfered
        
            printf("Sending file %s\n", full_path);
            FILE *file = fopen(full_path, "rb");
            if (file == NULL) {
                error("Error opening file");
            }

            // get file size
            fseek(file, 0, SEEK_END);
            file_size = ftell(file);
            fseek(file, 0, SEEK_SET);
            printf("file size; %lu\n", file_size);


            // set up file size in a 3 byte packet
            unsigned char sizeBytes[3];
            sizeBytes[0] = (file_size >> 16) & 0xFF;
            sizeBytes[1] = (file_size >> 8) & 0xFF;
            sizeBytes[2] = file_size & 0xFF;

            // send file size to client
            if (sendto(server_socket, sizeBytes, sizeof(sizeBytes), 0, (struct sockaddr*)&client_address, addr_len) == -1) {
                perror("Sendto error");
            }

            // read file then close it
            // char *file_buffer = malloc(file_size);
            file_buffer = malloc(file_size);
            if (file_buffer == NULL) {
                error("Memory allocation failed");
            }
            fread(file_buffer, 1, file_size, file);
            fclose(file);


            int bytes_sent;
            int remaining_bytes = file_size;
            char *ptr = file_buffer;



            // set alarm for 1.5 secs
            // signal(SIGALRM, sigalrm_handler); 
            // struct sigaction sa;
            // memset(&sa, 0, sizeof(sa));
            // sa.sa_handler = sigalrm_handler;    // set alarm handler
            // sigaction(SIGALRM, &sa, NULL);
            // // alarm(1.5);
            // alarm(TRANSFER_END_TIMEOUT);

            // ualarm(TRANSFER_END_TIMEOUT, 0);
            // setup signal handler for 1.5 seconds timeout
            struct sigaction sa;
            memset(&sa, 0, sizeof(sa));
            sa.sa_flags = SA_SIGINFO;
            sa.sa_sigaction = signal_handler;
            sigaction(SIGRTMIN, &sa, NULL);

            //setup timer for the 1.5 second timeout
            // timer_t timerid;
            struct sigevent sev;
            memset(&sev, 0, sizeof(sev));
            sev.sigev_notify = SIGEV_SIGNAL;
            sev.sigev_signo = SIGRTMIN;
            sev.sigev_value.sival_ptr = &timerid;
            timer_create(CLOCK_REALTIME, &sev, &timerid);






            // Set up SIGPOLL handler for handling recieved acks
            
            struct sigaction setup_action;
            setup_action.sa_handler = io_handler;
            setup_action.sa_flags = SA_NODEFER; // Prevent blocking
            if (sigaction(SIGPOLL, &setup_action, NULL) == -1) {
                perror("Sigaction");
                exit(EXIT_FAILURE);
            }

            // Set socket ownership and flags for asynchronous I/O
            if (fcntl(server_socket, F_SETOWN, getpid()) < 0) {
                perror("fcntl");
                exit(EXIT_FAILURE);
            }
            if (fcntl(server_socket, F_SETFL, O_RDONLY | FASYNC) < 0) {
                perror("fcntl");
                exit(EXIT_FAILURE);
            }


    // // set alarm for 1.5 secs
    //         signal(SIGALRM, sigalrm_handler); 
    //         // alarm(TRANSFER_END_TIMEOUT);
    //         ualarm(TRANSFER_END_TIMEOUT, 0);

             

            // send file packets
            while (transfer_mode == 1){ //remaining_bytes > 0) {
                // stay in transfer mode even if all packets are sent normally (because resending might happen)
                if (remaining_bytes > 0){
                    int bytes_to_send = remaining_bytes > MAX_PACKET_SIZE-1 ? MAX_PACKET_SIZE-1 : remaining_bytes;
                    memcpy(buffer + 1, ptr, bytes_to_send); // set payload of the packet
                    memcpy(buffer, &seq_num, 1);    // set sequence number of packet


                    // int random_number = rand() % 5; // Generate a random number 
                    // if (random_number == 0){    // only send the packet with 20 percent chance
                        printf("sending %d bytes with sequence number %d \n", bytes_to_send, seq_num);
                        bytes_sent = sendto(server_socket, buffer, bytes_to_send + 1, 0, (struct sockaddr *)&client_address, addr_len);
                            
                        if (bytes_sent < 0) {
                            error("Error sending packet");
                        }
                    // } else{
                    //     printf("not sending package no %d\n", seq_num);
                    // }
                    // usleep(100000);
                    ptr += bytes_to_send;
                    remaining_bytes -= bytes_to_send;
                    seq_num++;
                    
                    // set timer if it is last packet
                    if (remaining_bytes <= 0){
                        struct itimerspec its;
                        its.it_value.tv_sec = 1; // 1 second
                        its.it_value.tv_nsec = 500000000; // 0.5 seconds
                        its.it_interval.tv_sec = 0; // Non-repeating timer
                        its.it_interval.tv_nsec = 0;
                        timer_settime(timerid, 0, &its, NULL);
                    }
                }
            }
            printf("out of transfer mode\n");

            // Reversing SIGPOLL Handler Setup
            struct sigaction reset_action;
            reset_action.sa_handler = SIG_DFL;
            reset_action.sa_flags = 0; // Reset flags to default
            if (sigaction(SIGPOLL, &reset_action, NULL) == -1) {
                perror("Sigaction reset");
                exit(EXIT_FAILURE);
            }

            // Reversing Socket Configuration for Asynchronous I/O
            if (fcntl(server_socket, F_SETOWN, original_owner) < 0) {
                perror("fcntl reset");
                exit(EXIT_FAILURE);
            }
            if (fcntl(server_socket, F_SETFL, original_flags) < 0) {
                perror("fcntl reset");
                exit(EXIT_FAILURE);
            }

            // make socket back to blocking once transfer is over
            // set_socket_blocking(server_socket);

        }
    }

    // close(server_socket);
    return 0;
}


//  handler funciton for handling negative acks
void io_handler(int signal_recieved)
{
	int numbytes;	
    struct sockaddr_in their_addr;
    socklen_t addr_len = sizeof(their_addr);
    unsigned char buffer[2];

    // recieve negative ack from the server
    if ((numbytes=recvfrom(server_socket, buffer, 2, 0, (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
         exit(1);
    }

    // printf("resetting alarm\n");
    // alarm(TRANSFER_END_TIMEOUT);
    // signal(SIGALRM, sigalrm_handler); 
    // // printf("alarm is reset\n");
    //reset timer
    struct itimerspec its;
    its.it_value.tv_sec = 1; // 1 second
    its.it_value.tv_nsec = 500000000; // 0.5 seconds
    its.it_interval.tv_sec = 0; // Non-repeating timer
    its.it_interval.tv_nsec = 0;
    timer_settime(timerid, 0, &its, NULL);

    char buf[MAX_PACKET_SIZE];
    memcpy(buf, &buffer[0], 1); // set sequence number of packet

    char *ptr = file_buffer;
    int remaining_bytes = file_size;
    int bytes_to_send;
    // Find the section of file corresponding with the sequence number
    for (size_t i = 0; i < buffer[0]; i++){
            bytes_to_send = remaining_bytes > MAX_PACKET_SIZE-1 ? MAX_PACKET_SIZE-1 : remaining_bytes;
            ptr += bytes_to_send;
            remaining_bytes -= bytes_to_send;
    }

    memcpy(buf + 1, ptr, bytes_to_send);    // set the payload of the packet

    printf("resending %d bytes with sequence number %d \n", bytes_to_send, buffer[0]);
    int bytes_sent = sendto(server_socket, buf, bytes_to_send + 1, 0, (struct sockaddr *)&client_address, addr_len);
    if (bytes_sent < 0) {
        error("Error sending packet");
    }   
	return;
}


// alarm handler for detecting the end of file transer
void sigalrm_handler(int signum) {
    printf("Timer expired. No packets received for %.1f seconds.\n", 1.5);
    transfer_mode = 0;
    // close(server_socket);

}