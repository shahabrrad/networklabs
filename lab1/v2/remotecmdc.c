#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#define FIFO_NAME "myfifo"

void sigpipe_handler(int signum) {
    printf("SIGPIPE error with FIFO.\n");
}

int main() {
    int fd;
    char message[128];

    printf("Client is waiting to connect...\n");

    fd = open(FIFO_NAME, O_WRONLY);
    signal(SIGPIPE, sigpipe_handler);

    if (fd == -1) {     //  fail to open the FIFO
        perror("open");
        exit(EXIT_FAILURE);
    }

    printf("Connected to the server. Type a command (type 'exit' to quit):\n");

    while (1) {
        printf("?");
        fgets(message, sizeof(message), stdin); //  get the message to send to the server
        if ( (strlen(message) > 30) ){ 
            printf("command length exceeds maximum 30: %s\n", message);
            fflush(stdin);
        }else{
            if (strncmp(message, "exit", 4) == 0) {     //  exit program
                break;
            }

            ssize_t bytesWritten = write(fd, message, strlen(message));

            if (bytesWritten == -1) {
                perror("write");
                printf("An error occured while trying to write to FIFO\n");
                break; // Exit the loop and terminate the program on write error
            }
        }
    }

    close(fd);
    return 0;
}
