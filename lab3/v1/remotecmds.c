#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/wait.h>


#include "parser.h"
#include "ip.h"

#include <sys/socket.h>
#include <netinet/in.h>


#define PORT 8080
#define MAX_MESSAGE_LENGTH 1024
#define ALLOWED_PREFIX "128.10.112"




int main() {
    char *token;
    int k;
    int status;
    int len;


    int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int addrlen = sizeof(server_address);
    char message[MAX_MESSAGE_LENGTH];

    // Creating socket file descriptor
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Server address structure
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // // Print server IP address
    // struct sockaddr_in addr;
    // socklen_t addr_size = sizeof(struct sockaddr_in);
    // getsockname(server_socket, (struct sockaddr *)&addr, &addr_size);
    char server_ip[16]; // = inet_ntoa(addr.sin_addr);
    // printf("Server IP address: %s\n", server_ip);
    //  store server's ip in server_ip
    get_ip(server_ip);
    
    printf("pings %s %d %%\n", server_ip, PORT);

    printf("Server is waiting for a client...\n");
    // Listen for incoming connections
    if (listen(server_socket, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Accept incoming connections
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client_address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // // Check if the client IP matches the allowed prefix
    //     char *client_ip = inet_ntoa(client_address.sin_addr);
    //     if (strncmp(client_ip, ALLOWED_PREFIX, strlen(ALLOWED_PREFIX)) != 0) {
    //         printf("Connection rejected from IP: %s\n", client_ip);
    //         close(client_socket);
    //         perror("wrong connection");
    //         exit(EXIT_FAILURE);
    //         // continue;
    //     }


    printf("Client connected. Waiting for command...\n");

    while (1) {
        memset(message, 0, MAX_MESSAGE_LENGTH); // Clear command buffer
        int bytesRead = read(client_socket, message, MAX_MESSAGE_LENGTH);
        if (bytesRead <= 0) {
            perror("read");
            exit(EXIT_FAILURE);
        }

        message[bytesRead] = '\0';

        printf("message recieved with length %d \n", bytesRead);

        if (bytesRead > 0) {
                //  copy the message read 
                // char destination[bytesRead];
                // strncpy(destination, message, bytesRead);

                //  tokenize the message to get each command seperated by \n
                token = strtok(message, "\n");

                // Walk through other tokens
                while( token != NULL ) {
                    printf("Received message from client: %s\n", token);
                    if (strlen(token) > 30) {
                        printf("Ignoring message, exceeds 30 characters:\n%s\n", token);
                    }else{
                        len = strlen(token);
	                if(len == 1) 				// case: only return key pressed
	                    continue;
                    token[len] = '\0';			// case: command entered

	                // char **args = split_string(token);	// call the function form parser.c

  	                k = fork();
  	                if (k==0) {
  	                    // child code
    	                // if(execvp(args[0], args) == -1)	// if execution failed, terminate child
                        //     printf("error\n");
                        // Check if the client IP matches the allowed prefix
                        char *client_ip = inet_ntoa(client_address.sin_addr);
                        if (strncmp(client_ip, ALLOWED_PREFIX, strlen(ALLOWED_PREFIX)) != 0) {
                            printf("Command execution rejected from IP: %s\n", client_ip);
                            close(client_socket);

                            // continue;
                        }else{
                            execute_command(client_socket, token);
                        }
	  	                exit(1);
  	                }
  	                else {
  	                    // parent code 
	                    waitpid(k, &status, 0);		// block until child process terminates
  	                }
                    }
                    // 
                    token = strtok(NULL, "\n");     //  go for the next token or command
                }
            }
            
        }


    // close(fd);
    return 0;
}
