#ifndef SOCKET_UTILS_H
#define SOCKET_UTILS_H

void error(const char *msg);

int build_address(const char *const ip, const char *const port, int socktype, struct addrinfo **info);
int create_socket(const struct addrinfo *const info);
int bind_socket(const struct addrinfo *const info, const int sockfd);

#endif //SOCKET_UTILS_H
