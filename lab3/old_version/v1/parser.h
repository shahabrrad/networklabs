#ifndef PARSER_H
#define PARSER_H

// Function prototypes
char** split_string(char *buf);

void execute_command(int client_socket, char *command);

#endif