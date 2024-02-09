#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "ip.h"

void get_ip(char *target) {
    char host[256];
    char *IP;
    struct hostent *host_entry;

    // Get the host name
    gethostname(host, sizeof(host));

    // Get the host information
    host_entry = gethostbyname(host);

    // Convert the IP address to a string and print the results
    IP = inet_ntoa(*((struct in_addr*)host_entry->h_addr_list[0]));
    printf("Host IP: %s\n", IP);
    size_t ip_length = strlen(IP);
    strncpy(target, IP, ip_length);
    target[ip_length] = '\0'; // Ensure null-termination

    // return IP;
}