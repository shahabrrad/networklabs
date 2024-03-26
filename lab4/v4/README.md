
# CS 536 lab 1

In this project a client and a server are connected through a TCP socket. Each time a new client is connected to the server, the server forks to handle the client requeted command and stays listening for the next clients.

## Files:
- remotecmdc.c contains the code for the client program
- remotecmds.c contains the code for the server program
- parser.c contains the utility parser function which parses a command to get the arguments needed to execute the command
- parser.h is the header file to use parser.client
- Makefile to compile and link the files

## Running
To compile the code all you need to do is to use command `make`. After that the server is runnable with `./remotecmds.bin` command and the client can be started using `./remotecmdc.bin` command.

Consider that the ip of the server must be specified in the client code in the `SERVER_IP` constant.

## Constants
The ip address and the port of the server are defined in `SERVER_IP` and `PORT` constants in the client code.

The port of the server socket is defined in the `PORT` constant of the server code.

`ALLOWED_PREFIX` shows the allowed prefixes for the ip addresses that the server accepts commands from.

## Functions
The `split_string` function is accessible in parser.c file. This fucntion takes a string `buf` as input and splits it based on spaces. We use this function to get arguments which will be used to execute the command.

The `execute_command` function is accessible in parser.c file. This function is used for running commands and chekcing if these commands are allowed or not.

The code base for the server and the client are both under the `main()` function in their files.