# CS 536 lab 6

In this lab a routing server is configured by a TCP control connection with a approutec program. Based on these configurations, we can control the forwarding route of the ping messages from pingc to pings directrly throught the approutes program.

## Files:
- pingc.c contains the code for the client program
- pings.c contains the code for the server program
- approutes.c contains the code for the routing server
- approutec.c contains the code for routing configuration client.
- ip.c contains the utility  functions which extract the IP of the host, turn that IP into string
- ip.h is the header file to use ip.c
- Makefile to compile and link the files

## Running
To compile the code all you need to do is to use command `make`. First run the ping server server file by calling `./pings.bin <ping_server_ip> <ping_server_port>`. This will act similar to the second lab. Then we have to open and configure the routing servers. To run the tunneling server call `./approutes.bin <router-address> <port-num>`. The routing server will create a TCP socket and two UDP sockets and wait for configuraiton to come from the routing client. We use approutec for this purpose: `./approutec.bin`. With this we can configure as many routing servers as we want. The client program will promot `?` and gets inputs in the form of `<router-ip> <tcp-port> <next-hop-IP> <next-hop-port>`. The routing client will send next hop ip and port to the routing server through the tcp socket. Once all routing servers are configured, we can run the ping client and send messages to the first routing server. Consider that the `next-hop` address of the last router should be the ping server.

## Parameters

#### Globals (constants.h)
- The size of the message is specified by the `BUFFER_SIZE` constant
- The number of bytes used for sequence number is specified by `SEQ_SIZE` constant
- The number of bytes used for command is specified by the `COMMAND_SIZE` constant

#### Ping Server (pings.c)
- The port number used by the server is specified by the `DEFAULT_PORT` constant
- The number of bind attempt possible for the server is specified in `MAXIMUM_BINDS` constant
- The amount of time that the server should wait if it is pinged with command number 1 is specified in `WAIT_TIME` constant which is equal to 555 miliseconds

#### Ping Client (pingc.c)
- The number of pings to be sent by the client is specified by the `MAX_ATTEMPTS` constant
- The initial sequence number of packet is specified in `INITIAL_SEQ` constant
- The amount of time that the client should wait before timing out for ping is specified by `TIMEOUT` constant
- The control byte value is specified by `CONTROL` constant

#### Routing Server (approutes.c)

- The maximum number of binding attempts for any of the UDP sockets is specified in the `MAX_BINDS` constant

## Functions
The `get_ip` function is accessible in ip.c file. This fucntion finds the ip of the current host (either a client or a server) and turns it into a string.

The `convertSockaddrToIPString` function is accessible in ip.c file. This fucntion turns the ip of the client into a string given its socket address. This function is used in the server program.

The code base for the server and the client are both under the `main()` function in their files.