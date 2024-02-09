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
