# CS 536 lab 2 v1

In this lab a server process and a client process are created and connected through UDP sockets. Then the client application pings the server 10 times and terminates.

## Files:
- pingc.c contains the code for the client program
- pings.c contains the code for the server program
- ip.c contains the utility  functions which extract the IP of the host, turn that IP into string
- parser.h is the header file to use ip.c
- Makefile to compile and link the files

## Running
To compile the code all you need to do is to use command `make`. First run the server file by calling `./pings.bin`. Server will create a socket, bind and then print the ip and port of the server in the format: `% pings 128.10.112.135 44444`.
Then you can run the client by calling `./pingc.bin <server_ip> <server_port>` in which server_ip and server_port are the ip and port printed in the output of the server.

## Parameters

#### Globals (constants.h)
- The size of the message is specified by the `BUFFER_SIZE` constant
- The number of bytes used for sequence number is specified by `SEQ_SIZE` constant
- The number of bytes used for command is specified by the `COMMAND_SIZE` constant

#### Server (pings.c)
- The port number used by the server is specified by the `DEFAULT_PORT` constant
- The number of bind attempt possible for the server is specified in `MAXIMUM_BINDS` constant
- The amount of time that the server should wait if it is pinged with command number 1 is specified in `WAIT_TIME` constant which is equal to 555 miliseconds

#### Client (pingc.c)
- The number of pings to be sent by the client is specified by the `MAX_ATTEMPTS` constant
- The initial sequence number of packet is specified in `INITIAL_SEQ` constant
- The amount of time that the client should wait before timing out for ping is specified by `TIMEOUT` constant
- The control byte value is specified by `CONTROL` constant

## Functions
The `get_ip` function is accessible in ip.c file. This fucntion finds the ip of the current host (either a client or a server) and turns it into a string.

The `convertSockaddrToIPString` function is accessible in ip.c file. This fucntion turns the ip of the client into a string given its socket address. This function is used in the server program.

The code base for the server and the client are both under the `main()` function in their files.