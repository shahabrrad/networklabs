
# CS 536 lab 1

In this project a client and a server are connected through a FIFO buffer

## Files:
- remotecmdc.c contains the code for the client program
- remotecmds.c contains the code for the server program
- parser.c contains the utility parser function which parses a command to get the arguments needed to execute the command
- parser.h is the header file to use parser.client
- Makefile to compile and link the files

## Running
To compile the code all you need to do is to use command `make`. After that the server is runnable with `./remotecmds.bin` command and the client can be started using `./remotecmdc.bin` command.

## Functions
The `split_string` function is accessible in parser.c file. This fucntion takes a string `buf` as input and splits it based on spaces. We use this function to get arguments which will be used to execute the command.

The code base for the server and the client are both under the `main()` function in their files.