#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "parser.h"

#define FIFO_NAME "myfifo"


int main() {
    int fd;
    char message[128];
    char *token;
    int k;
    int status;
    int len;

    // Create the FIFO if it doesn't exist
    mkfifo(FIFO_NAME, 0666);

    printf("Server is waiting for a client...\n");
    fd = open(FIFO_NAME, O_RDONLY);

    if (fd == -1) {     //  fail to open FIFO
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Client connected. Waiting for command...\n");

    while (1) {

        int bytesRead = read(fd, message, sizeof(message));
        if (bytesRead > 0) {
                //  copy the message read 
                char destination[bytesRead];
                strncpy(destination, message, bytesRead);

                //  tokenize the message to get each command seperated by \n
                token = strtok(destination, "\n");

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

	                char **args = split_string(token);	// call the function form parser.c

  	                k = fork();
  	                if (k==0) {
  	                    // child code
    	                if(execvp(args[0], args) == -1)	// if execution failed, terminate child
                            printf("error\n");
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
            // }
            
        }
    }

    close(fd);
    return 0;
}
