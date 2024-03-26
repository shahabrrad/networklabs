#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include "parser.h"

//  This is a general purpose function so we use "words" to refer to args
#define MAX_WORDS 100  // Maximum number of args we expect
#define MAX_WORD_LEN 30  // Maximum length of each arg

char **split_string(char *buf) {
    char **result = malloc(MAX_WORDS * sizeof(char*));  // Allocate memmory for array of splitted string
    if (result == NULL) {
        perror("malloc failed");
        return NULL;
    }

    int word_count = 0;
    char *word = strtok(buf, " ");  // Use strtok to find the first arg in the string
    while (word != NULL) {  // Continue until there are no more arg
        if (word_count >= MAX_WORDS) {
            break;
        }

        result[word_count] = malloc(strlen(word) + 1);  // Allocate memmory for each arg +1 for null terminator
        if (result[word_count] == NULL) {
            perror("malloc failed");
            // Free already allocated memory
            for (int i = 0; i < word_count; i++) {
                free(result[i]);
            }
            free(result);
            return NULL;
        }

        // Copy the arg into the allocated space
        strcpy(result[word_count], word);
        word_count++;
        word = strtok(NULL, " ");   // Find the next arg
    }

    return result;
}

void execute_command(int client_socket, char *command) {


    char *args[1024]; // Maximum command length assumed
    
    // Tokenizing the command
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // Last argument must be NULL for execvp

    // Check if the command is "ls" or "date"
    if (strcmp(args[0], "ls") != 0 && strcmp(args[0], "date") != 0) {
        // Ignore any other command
        char error_message[] = "command failed: only ls and date are allowed";
        ssize_t bytesWritten = write(client_socket, error_message, strlen(error_message) + 1); // Include '\0' in the message
        if (bytesWritten == -1) {
                perror("write");
            }
        // exit(EXIT_FAILURE);
        return;
    }

    // Check the number of arguments for "ls"
    if (strcmp(args[0], "ls") == 0 && i > 4) {
        // Ignore "ls" command with more than 3 arguments
        char error_message[] = "command failed: number of arguments for ls exceeds maximum";
        ssize_t bytesWritten = write(client_socket, error_message, strlen(error_message) + 1); // Include '\0' in the message
        if (bytesWritten == -1) {
                perror("write");
            }
        // exit(EXIT_FAILURE);
        return;
    }
    // Execute the command
    if (execvp(args[0], args) < 0) {
        perror("execvp");
        char error_message[] = "command failed";
        ssize_t bytesWritten = write(client_socket, error_message, strlen(error_message) + 1); // Include '\0' in the message
        if (bytesWritten == -1) {
                perror("write");
            }
        exit(EXIT_FAILURE);
    }
}
