
# CS 536 lab 2 v3

In this lab a file transfer server and client are used to transfer a file from the server to the client. The server runs indefinitly and the client terminates once the transfer process is finished.

The server files are stored in the `/tmp/server/` in local directory. The client saves the transfered file in the `/tmp/client/` folder in local directory. This helps us run the client and server programs on the same machine.

You will have to change packet size manually within code.

## Files:
- ssftp.c contains the code for the client program
- ssftpd.c contains the code for the server program
- ip.c contains the utility  functions which extract the IP of the host, turn that IP into string
- parser.h is the header file to use ip.c
- Makefile to compile and link the files
- There are 3 files with different sizes in `tmp/server/` that can be used for transfer experiment. Once these are transfered, they will be saved in `tmp/client`. These files are named 2560, 25600 and 256000 based on their size in bytes.

## Running
To compile the code all you need to do is to use command `make`. First run the server file by calling `./ssftpd.bin <server_ip> <port_number>`. The ip in this command is reduntant since no matter what, the ip of server is the ip of the machine it is running on. The port will be assigned in a similar way to the ping application: 10 attempts with increasing port number. Server will create a socket, bind and then print the ip and port of the server in the format: `% pings 128.10.112.135 50001`.

Then you can run the client by calling `./ssftp.bin <file_name> <server_ip> <server_port> <client_ip> <packet_size>`.  In a similar manner to the server, the <client_ip> is reduntant here. Also the packet size is not set by the argument and it is reduntant because in fact the packet size is set by a shared constant parameter in the header file.

The client will request the server to resend any lost packet and the server will finish the file transmition once no new request is sent. After this the server can recieve new file transfer requests from new clients.
## Parameters

#### Globals (constants.h)
- The size of the transfer packets is specified by the `MAX_PACKET_SIZE` constant
- The number of bytes used for file name is specified by `FILENAME_LENGTH` constant


#### Server (ssftp.c)
- The port number used by the server is specified by the `DEFAULT_PORT` constant which is used for testing and is replaced by the number specified while running the server
- The number of bind attempt possible for the server is specified in `MAXIMUM_BINDS` constant
- The amount of time that should pass since the last negative ack recieved to know that the file transfer has finished is specified by the `TRANSFER_END_TIMEOUT` constant in seconds.

#### Client (ssftp.c)
- The amount of time that the client should wait before timing out for server responce is specified by `TIMEOUT` constant
- The time interval between each alarm for resending lost packets is specified in `TIMEOUT_USEC` in milliseconds.

## Functions
The `get_ip` function is accessible in ip.c file. This fucntion finds the ip of the current host (either a client or a server) and turns it into a string.

The `convertSockaddrToIPString` function is accessible in ip.c file. This fucntion turns the ip of the client into a string given its socket address. This function is used in the server program.

The `error` and `alarm_handler` functions are for handling errors and alarms.

The `remove_trailing_Z` function is used for removing trailing Zs on server side.

The `alarm_handler_lostpacket` function in client code is used to check for lost packets and resend them to the server. This function is called cocurrently every 100 milliseconds. This interval can be changed by the `TIMEOUT_USEC` constant. 

The `io_handler` function in the server handles negative ack resend requests sent by the client after entering transfer mode.

The `signal_handler` function in the server detects the end of file transfer by the fact that a new resend request has not been recieved from the client in the past 1.5 seconds. This time interval is set by the `TRANSFER_END_TIMEOUT` constant.

The code base for the server and the client are both under the `main()` function in their files.