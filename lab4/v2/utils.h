#ifndef UTILS_H
#define UTILS_H

// Function prototypes
void get_ip(char *target);

char *convertSockaddrToIPString(const struct sockaddr *addr);

#endif