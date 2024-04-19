#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <errno.h>


#define MAX_BINDS 100


int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <router-address> <port-num>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int initial_port = atoi(argv[2]);

    int server_fd, tcp_socket, valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);


    // --------------- TCP socket -----------
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
    unsigned short tcp_port = initial_port;
    while(1){
        address.sin_port = htons(tcp_port);
        // Forcefully attaching socket to the port
        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
            perror("bind failed");
            // exit(EXIT_FAILURE);
        }else{
            break;
        }
    }
    printf("TCP port: %d\n", tcp_port);
    tcp_port ++;
    // -------------------------------------------

    // ------------ First UDP socket ---------------
    int udp_sockfd_in; // socket to connect to pingc client
    struct sockaddr_in udp_addr_in;
    unsigned short port_number_in = tcp_port;

    udp_sockfd_in = socket(AF_INET, SOCK_DGRAM, 0);
    // Bind the UDP socket
    memset(&udp_addr_in, 0, sizeof(udp_addr_in));
    udp_addr_in.sin_family = AF_INET;
    udp_addr_in.sin_addr.s_addr = INADDR_ANY;
    udp_addr_in.sin_port = htons(port_number_in);

    // printf("child number %d, udp_addr port_number %d\n", tunnelindex, port_number);

    while (bind(udp_sockfd_in, (struct sockaddr *)&udp_addr_in, sizeof(udp_addr_in)) < 0) {
        port_number_in++; // TODO change the port selction here.
        if (port_number_in > initial_port + MAX_BINDS) { // Limsit retries to 100 ports
            fprintf(stderr, "Failed to bind UDP socket to available port.\n");
            close(udp_sockfd_in);
            exit(EXIT_FAILURE);
        }
        udp_addr_in.sin_port = htons(port_number_in);
    }
    printf("First UDP port (in): %d\n", port_number_in);
    //  ----------------------------------------------
    port_number_in ++;

    // -------- second UDP socket -----------
    int udp_sockfd_out; // socket to connect to pingc client
    struct sockaddr_in udp_addr_out;
    unsigned short port_number_out = port_number_in;

    udp_sockfd_out = socket(AF_INET, SOCK_DGRAM, 0);
    // Bind the UDP socket
    memset(&udp_addr_out, 0, sizeof(udp_addr_out));
    udp_addr_out.sin_family = AF_INET;
    udp_addr_out.sin_addr.s_addr = INADDR_ANY;
    udp_addr_out.sin_port = htons(port_number_out);

    // printf("child number %d, udp_addr port_number %d\n", tunnelindex, port_number);

    while (bind(udp_sockfd_out, (struct sockaddr *)&udp_addr_out, sizeof(udp_addr_out)) < 0) {
        port_number_out++;
        if (port_number_out > initial_port + MAX_BINDS) { // Limit retries to 100 ports
            fprintf(stderr, "Failed to bind UDP socket to available port.\n");
            close(udp_sockfd_out);
            exit(EXIT_FAILURE);
        }
        udp_addr_out.sin_port = htons(port_number_out);
    }
    printf("Second UDP port (out): %d\n", port_number_out);

    // ------------------------------------
    
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connections...\n");
    if ((tcp_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // ---------- TODO change parameter names to next hop --------
     // Read 4 bytes for the final destination's IPv4 address
        uint32_t next_hop_ip;
        valread = read(tcp_socket, &next_hop_ip, sizeof(uint32_t));
        if (valread < 0) {
            perror("read");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }

        // Convert to network byte order (big-endian to little-endian)
        // next_hop_ip = ntohl(next_hop_ip);
        printf("Next hop IPv4 address: %s\n", inet_ntoa(*(struct in_addr*)&next_hop_ip));

        // Read 2 bytes for the final destination's port number
        uint16_t next_hop_port;
        valread = read(tcp_socket, &next_hop_port, sizeof(uint16_t));
        if (valread < 0) {
            perror("read");
            close(tcp_socket);
            exit(EXIT_FAILURE);
        }

        // Convert to network byte order (big-endian to little-endian)
        next_hop_port = ntohs(next_hop_port);
        printf("Next Hop port number: %d\n", next_hop_port);
    // --------------------------



    // Create a second UDP socket for forwarding the payload
    struct sockaddr_in forward_dst_addr; 
    struct sockaddr_in forward_src_addr; 
    socklen_t forward_dst_addr_len = sizeof(forward_dst_addr);
    socklen_t forward_src_addr_len = sizeof(forward_src_addr);

    // Bind the second UDP socket
    memset(&forward_dst_addr, 0, sizeof(forward_dst_addr));
    forward_dst_addr.sin_family = AF_INET;
    forward_dst_addr.sin_port = htons(next_hop_port);
    forward_dst_addr.sin_addr.s_addr = next_hop_ip;


            //initialize the select process
            fd_set readfds;
            int max_fd = (udp_sockfd_out > udp_sockfd_in) ? udp_sockfd_out : udp_sockfd_in;
            // max_fd = (tcp_socket > max_fd) ? tcp_socket : max_fd;
            
            char *src_ip;
            unsigned int src_port;
            
            while (1) {
                FD_ZERO(&readfds);
                FD_SET(udp_sockfd_out, &readfds);
                FD_SET(udp_sockfd_in, &readfds);
                // FD_SET(tcp_socket, &readfds);
                printf("waiting for packets...\n");

                int activity = select(max_fd + 1, &readfds, NULL, NULL, NULL);
                if ((activity < 0) && (errno != EINTR)) {
                    perror("select");
                    close(udp_sockfd_in);
                    close(udp_sockfd_out);
                    exit(EXIT_FAILURE);
                }

                // Handle activity on UDP socket for communication with client
                if (FD_ISSET(udp_sockfd_in, &readfds)) {
                    char buffer[1024];
                    ssize_t recv_len = recvfrom(udp_sockfd_in, buffer, sizeof(buffer), 0, (struct sockaddr *)&forward_src_addr, &forward_src_addr_len);
                    if (recv_len < 0) {
                        perror("recvfrom");
                        close(udp_sockfd_in);
                        close(udp_sockfd_out);
                        exit(EXIT_FAILURE);
                    }
                    
                    // Extracting IP and port
                    src_ip = inet_ntoa(forward_src_addr.sin_addr);
                    src_port = ntohs(forward_src_addr.sin_port);
                    // Printing IP and port
                    printf("Received packet from source (in) IP: %s, Port: %u\n", src_ip, src_port);

                        // Forward UDP packet to the destination
                        if (sendto(udp_sockfd_out, buffer, recv_len, 0, (struct sockaddr *)&forward_dst_addr, forward_dst_addr_len) < 0) {
                            perror("sendto");
                            close(udp_sockfd_in);
                            close(udp_sockfd_out);
                            exit(EXIT_FAILURE);
                        }

                    src_ip = inet_ntoa(forward_dst_addr.sin_addr);
                    src_port = ntohs(forward_dst_addr.sin_port);
                    printf("sent package to destination (out) IP: %s, Port: %u\n", src_ip, src_port);
                    
                    
                }

                // Handle activity on the forward UDP socket for receiving responses from final destination
                if (FD_ISSET(udp_sockfd_out, &readfds)) {
                    char buffer[1024];
                    ssize_t recv_len = recvfrom(udp_sockfd_out, buffer, sizeof(buffer), 0, (struct sockaddr *)&forward_dst_addr, &forward_dst_addr_len);
                    if (recv_len < 0) {
                        perror("recvfrom");
                        close(udp_sockfd_in);
                        close(udp_sockfd_out);
                        exit(EXIT_FAILURE);
                    }

                    // Extracting IP and port
                    src_ip = inet_ntoa(forward_dst_addr.sin_addr);
                    src_port = ntohs(forward_dst_addr.sin_port);
                    printf("Received packet from destination (out) IP: %s, Port: %u\n", src_ip, src_port);



                    // Forward response to the client
                    if (sendto(udp_sockfd_in, buffer, recv_len, 0, (struct sockaddr *)&forward_src_addr, forward_src_addr_len) < 0) {
                        perror("sendto");
                        close(udp_sockfd_out);
                        close(udp_sockfd_in);
                        exit(EXIT_FAILURE);
                    }

                    src_ip = inet_ntoa(forward_src_addr.sin_addr);
                    src_port = ntohs(forward_src_addr.sin_port); 
                    // Printing IP and port
                    printf("sent package to source (in) IP: %s, Port: %u\n", src_ip, src_port);
                }
                
            }

            close(server_fd);
            // handle_connection(tcp_socket, tunnelindex, tabentry);
            exit(EXIT_SUCCESS);
        

        // Close the socket
        close(tcp_socket);
    // }
    return 0;
}
