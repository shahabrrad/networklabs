#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>

#define SECRET_KEY "mysecret" // Constants for hard coding
#define PORT 8080
#define MAX_SESSIONS 6
#define BASE_CHILD_PORT 60000
#define TUNNEL_BASE_PORT 64000
#define MAX_BINDS 100

struct forwardtab {
    unsigned long srcaddress;
    unsigned short srcport;
    unsigned long dstaddress;
    unsigned short dstport;
    unsigned short tunnelsport;
};


int main(int argc, char *argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <server_port> <secret_key>\n", argv[0]);
        return 1; // Return an error code
    }
    char *secret_key = argv[3];
    int server_port = atoi(argv[2]);

    int server_fd, tcp_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[16] = {0}; // Maximum size for header data

    // initialize forwarding tab struct
    struct forwardtab tabentry[MAX_SESSIONS] = {0};

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &(int){1}, sizeof(int)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(server_port);

    // Forcefully attaching socket to the port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    while(1) {
        printf("Waiting for connections...\n");
        if ((tcp_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }


        // Check if we have room for a new forwarding session
        int tunnelindex = -1;
        for (int i = 0; i < MAX_SESSIONS; ++i) {
            if (tabentry[i].srcaddress == 0) {
                tunnelindex = i;
                break;
            }
        }

        // If no available tunnel slots, ignore the request
        if (tunnelindex == -1) {
            printf("Maximum active forwarding sessions reached. Ignoring additional client request.\n");
            close(tcp_socket);
            continue;
        }



        // Read the first byte to check for a new connection request
        valread = read(tcp_socket, buffer, 1);
        if (valread < 0) {
            perror("read");
            close(tcp_socket);
            continue;
        }

        // Check if the first byte is 'c'
        if (buffer[0] != 'c') {
            printf("Invalid connection request. Closing socket.\n");
            close(tcp_socket);
            continue;
        }

        // Read the next 8 bytes for the secret key
        valread = read(tcp_socket, buffer, 8);
        if (valread < 0) {
            perror("read");
            close(tcp_socket);
            continue;
        }
        // printf("buffer value %s \n", buffer);
        
        // Check if the secret key matches
        if (strncmp(buffer, secret_key, 8) != 0) {
            printf("Invalid secret key. Closing socket.\n");
            close(tcp_socket);
            continue;
        }

        // Read 4 bytes for the final destination's IPv4 address
        uint32_t final_dest_ip;
        valread = read(tcp_socket, &final_dest_ip, sizeof(uint32_t));
        if (valread < 0) {
            perror("read");
            close(tcp_socket);
            continue;
        }

        // Convert to network byte order (big-endian to little-endian)
        // final_dest_ip = ntohl(final_dest_ip);
        printf("Final destination IPv4 address: %s\n", inet_ntoa(*(struct in_addr*)&final_dest_ip));

        // Read 2 bytes for the final destination's port number
        uint16_t final_dest_port;
        valread = read(tcp_socket, &final_dest_port, sizeof(uint16_t));
        if (valread < 0) {
            perror("read");
            close(tcp_socket);
            continue;
        }

        // Convert to network byte order (big-endian to little-endian)
        final_dest_port = ntohs(final_dest_port);
        printf("Final destination port number: %d\n", final_dest_port);

        // Read the final 4 bytes for the source IPv4 address
        uint32_t source_ip;
        valread = read(tcp_socket, &source_ip, sizeof(uint32_t));
        if (valread < 0) {
            perror("read");
            close(tcp_socket);
            continue;
        }

        // Convert to network byte order (big-endian to little-endian)
        // source_ip = ntohl(source_ip);
        printf("Source IPv4 address: %s\n", inet_ntoa(*(struct in_addr*)&source_ip));


        // Update tabentry for the new tunneling session
        tabentry[tunnelindex].srcaddress = source_ip; //final_dest_ip;
        // tabentry[tunnelindex].srcport = final_dest_port;
        tabentry[tunnelindex].dstaddress = final_dest_ip;
        tabentry[tunnelindex].dstport = final_dest_port;
        // tabentry[tunnelindex].tunnelsport = ntohs(address.sin_port);


        // Fork a child process to handle packet forwarding
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            int udp_sockfd; // socket to connect to pingc client
            struct sockaddr_in udp_addr;
            unsigned short port_number = BASE_CHILD_PORT;

            udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
            // Bind the UDP socket
            memset(&udp_addr, 0, sizeof(udp_addr));
            udp_addr.sin_family = AF_INET;
            udp_addr.sin_addr.s_addr = INADDR_ANY;
            udp_addr.sin_port = htons(port_number);

            // printf("child number %d, udp_addr port_number %d\n", tunnelindex, port_number);

            while (bind(udp_sockfd, (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
                port_number++;
                if (port_number > BASE_CHILD_PORT + MAX_BINDS) { // Limit retries to 100 ports
                    fprintf(stderr, "Failed to bind UDP socket to available port.\n");
                    close(udp_sockfd);
                    exit(EXIT_FAILURE);
                }
                udp_addr.sin_port = htons(port_number);
            }

            printf("child number %d, udp_addr port selected: %d \n", tunnelindex, port_number);
            // tabentry[tunnelindex].tunnelsport = port_number;
            tabentry[tunnelindex].srcport = port_number;
            unsigned short udp_port_n = htons(port_number);
            if (write(tcp_socket, &udp_port_n, sizeof(unsigned short)) < 0) {
                perror("write");
                close(udp_sockfd);
                exit(EXIT_FAILURE);
            }

            // Create a second UDP socket for forwarding the payload
            int forward_sockfd; // socket to connect to pings server
            struct sockaddr_in forward_dst_addr; 
            struct sockaddr_in forward_src_addr; 
            socklen_t forward_dst_addr_len = sizeof(forward_dst_addr);
            socklen_t forward_src_addr_len = sizeof(forward_src_addr);
            unsigned short tunnel_port_number = TUNNEL_BASE_PORT;


            forward_sockfd = socket(AF_INET, SOCK_DGRAM, 0);

            // Bind the second UDP socket
            memset(&forward_dst_addr, 0, sizeof(forward_dst_addr));
            forward_dst_addr.sin_family = AF_INET;
            // forward_dst_addr.sin_addr.s_addr = INADDR_ANY;
            // forward_dst_addr.sin_port = htons(tunnel_port_number);
            forward_dst_addr.sin_port = htons(tabentry[tunnelindex].dstport);
            forward_dst_addr.sin_addr.s_addr = tabentry[tunnelindex].dstaddress;

            memset(&forward_src_addr, 0, sizeof(forward_src_addr));
            forward_src_addr.sin_family = AF_INET;
            forward_src_addr.sin_addr.s_addr = tabentry[tunnelindex].srcaddress;
            
            printf("child number %d, forward_dst: %s %d \n", tunnelindex, inet_ntoa(*(struct in_addr*)&tabentry[tunnelindex].dstaddress), tabentry[tunnelindex].dstport);

            // address to communicate with client
            struct sockaddr_in clientaddr;
            memset(&clientaddr, 0, sizeof(clientaddr));
            clientaddr.sin_family = AF_INET;
            clientaddr.sin_port = htons(tunnel_port_number); // Client port number
            clientaddr.sin_addr.s_addr = htonl(INADDR_ANY);

            // printf("child number %d, forward_dst: %d %d \n", tunnelindex, tabentry[tunnelindex].dstaddress, tabentry[tunnelindex].dstport);


            while (bind(forward_sockfd, (struct sockaddr *)&clientaddr, sizeof(clientaddr)) < 0) {
                // perror("binding tunnel udp");
                tunnel_port_number++;
                clientaddr.sin_port = htons(tunnel_port_number);
                if (tunnel_port_number > TUNNEL_BASE_PORT + MAX_BINDS) { // Limit retries to 100 ports
                    fprintf(stderr, "Failed to bind tunneling UDP socket to available port.\n");
                    close(udp_sockfd);
                    close(forward_sockfd);
                    exit(EXIT_FAILURE);
                }
                clientaddr.sin_port = htons(tunnel_port_number);
            }

            printf("child number %d, clientaddr port: %d \n", tunnelindex, tunnel_port_number);

             // Store the tunnel port number in tabentry
            tabentry[tunnelindex].tunnelsport = htons(tunnel_port_number);

            //initialize the select process
            fd_set readfds;
            int max_fd = (udp_sockfd > forward_sockfd) ? udp_sockfd : forward_sockfd;
            max_fd = (tcp_socket > max_fd) ? tcp_socket : max_fd;
            
            
            while (1) {
                FD_ZERO(&readfds);
                FD_SET(udp_sockfd, &readfds);
                FD_SET(forward_sockfd, &readfds);
                FD_SET(tcp_socket, &readfds);

                int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
                if ((activity < 0) && (errno != EINTR)) {
                    perror("select");
                    close(udp_sockfd);
                    close(forward_sockfd);
                    exit(EXIT_FAILURE);
                }

                // Handle activity on UDP socket for communication with client
                if (FD_ISSET(udp_sockfd, &readfds)) {
                    char buffer[1024];
                    ssize_t recv_len = recvfrom(udp_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&forward_src_addr, &forward_src_addr_len);
                    if (recv_len < 0) {
                        perror("recvfrom");
                        close(udp_sockfd);
                        close(forward_sockfd);
                        exit(EXIT_FAILURE);
                    }
                    if (forward_src_addr.sin_addr.s_addr == tabentry[tunnelindex].srcaddress){
                        tabentry[tunnelindex].srcport = forward_src_addr.sin_port;

                        // Forward UDP packet to the destination
                        if (sendto(forward_sockfd, buffer, recv_len, 0, (struct sockaddr *)&forward_dst_addr, forward_dst_addr_len) < 0) {
                            perror("sendto");
                            close(udp_sockfd);
                            close(forward_sockfd);
                            exit(EXIT_FAILURE);
                        }
                    }else{
                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &forward_src_addr.sin_addr, ip, sizeof(ip));
                        printf("invalid packet sender in child %d. expected: %s got: %s \n", tunnelindex, inet_ntoa(*(struct in_addr*)&tabentry[tunnelindex].srcaddress), ip);
                    }
                    
                }

                // Handle activity on the forward UDP socket for receiving responses from final destination
                if (FD_ISSET(forward_sockfd, &readfds)) {
                    char buffer[1024];
                    ssize_t recv_len = recvfrom(forward_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&forward_dst_addr, &forward_dst_addr_len);
                    if (recv_len < 0) {
                        perror("recvfrom");
                        close(udp_sockfd);
                        close(forward_sockfd);
                        exit(EXIT_FAILURE);
                    }

                    // set the forward src adress before sending just to be sure
                    memset(&forward_src_addr, 0, sizeof(forward_src_addr));
                    forward_src_addr.sin_family = AF_INET;
                    forward_src_addr.sin_addr.s_addr = tabentry[tunnelindex].srcaddress;
                    forward_src_addr.sin_port = tabentry[tunnelindex].srcport;

                    // Forward response to the client
                    if (sendto(udp_sockfd, buffer, recv_len, 0, (struct sockaddr *)&forward_src_addr, forward_src_addr_len) < 0) {
                        perror("sendto");
                        close(udp_sockfd);
                        close(forward_sockfd);
                        exit(EXIT_FAILURE);
                    }
                }

                // Handle activity on the TCP socket for potential communication with the parent
                if (FD_ISSET(tcp_socket, &readfds)) {
                    char secret_key[9];
                    valread = read(tcp_socket, &secret_key, sizeof(secret_key));
                    if (valread < 0) {
                        perror("recv");
                        close(udp_sockfd);
                        close(forward_sockfd);
                        exit(EXIT_FAILURE);
                    } else if (valread == 8) {
                        secret_key[8] = '\0';
                        if (strcmp(secret_key, "mysecret") == 0) {
                            // Close TCP socket
                            close(tcp_socket);
                            // Free up the session entry in tabentry[]
                            tabentry[tunnelindex].srcaddress = 0;
                            // Terminate child process
                            exit(EXIT_SUCCESS);
                        }
                    }
                    // if (valread < 0) {
                    //     perror("read");
                    //     close(tcp_socket);
                    //     continue;
                    // }
                    // // There should be no activity on the TCP socket in the child process
                    // // The parent process should communicate the required information to the child
                    // if (valread > 0){
                    //     fprintf(stderr, "Unexpected activity on TCP socket in child process.\n");
                    // }
                    
                }
            }



            close(server_fd);
            // handle_connection(tcp_socket, tunnelindex, tabentry);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            // Fork error
            perror("fork");
            close(tcp_socket);
        } else {
            // Parent process
            close(tcp_socket);
        }

        // Close the socket
        close(tcp_socket);
    }
    return 0;
}
