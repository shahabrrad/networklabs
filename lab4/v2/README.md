# CS 536 lab 4 v2

In this lab a tunneling server is configured by a TCP control connection with a tunnelc program. Based on these configurations, we can forward the ping messages from pingc to pings directrly throught the tunnels program.

## Files:
- pingc.c contains the code for the client program
- pings.c contains the code for the server program
- tunnels.c contains the code for the tunneling server
- tunnelc.c contains the code for tunnel configuration client.
- ip.c contains the utility  functions which extract the IP of the host, turn that IP into string
- parser.h is the header file to use ip.c
- Makefile to compile and link the files

## Running
To compile the code all you need to do is to use command `make`. First run the ping server server file by calling `./pings.bin <ping_server_ip> <ping_server_port>`. This will act similar to the second lab. Then we have to open and configure the tunneling server. To run the tunneling server call `./tunnels.bin <tunnel_server_ip> <tunnel_server_port> <secret_key>`. We will use this secret key to connect tunnelc to it and configure the tunneling server. We use tunnelc for this purpose: `./tunnelc.bin <tunnel_server_ip> <tunnel_server_port> <secret_key> <ping_client_ip> <ping_server_ip> <ping_server_port>`. This creates a UDP socket for connecting ping client to tunneling server and sends the designated port to tunnelc.
We can now 

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

#### Tunnling Server (tunnels.c)
- The total number of tunneling sessions that can be managed are specified by the `MAX_SESSIONS` constant
- The initial port that the child assign to the UDP socket that will communicate with the ping client is specified by the `BASE_CHILD_PORT` constant.
- The initial port that the child will use to communicate with the ping server is specified by the `TUNNEL_BASE_PORT` constant.
- The maximum number of binding attempts for any of the UDP sockets is specified in the `MAX_BINDS` constant

## Functions
The `get_ip` function is accessible in ip.c file. This fucntion finds the ip of the current host (either a client or a server) and turns it into a string.

The `convertSockaddrToIPString` function is accessible in ip.c file. This fucntion turns the ip of the client into a string given its socket address. This function is used in the server program.

The code base for the server and the client are both under the `main()` function in their files.