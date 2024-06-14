#include "../inc/Network.h"

#define PORT "12306"
#define BACKLOG  10

int tcp_listen(void) {
    struct addrinfo hints, *res, *p;

    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    // wildcard address

    int rv;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
        error(1, rv, "getaddrinfo: %s", gai_strerror(rv));
    }

    // loop through all the results and bind to the first we can
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            error(0, errno, "socket failed");
            continue;
        }

        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
            error(1, errno, "setsockopt");
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            error(0, errno, "bind");
            continue;
        }

        break;
    }

    freeaddrinfo(res); // all done with this structure

    if (p == NULL) {
        error(1, 0, "failed to bind");
    }

    if (listen(sockfd, BACKLOG) == -1) {
        error(1, errno, "listen");
    }

    return sockfd;
}

int tcp_connect(const char* host, const char* service)
{
    struct addrinfo hints, *res, *p;
    // fill out hints
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    if ((rv = getaddrinfo(host, service, &hints, &res)) != 0) {
        error(1, 0, "getaddrinfo: %s", gai_strerror(rv));
    }

    // loop through all the results and connect to the first we can
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            error(0, errno, "socket failed");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            error(0, errno, "connect failed");
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        error(1, 0, "connect to %s failed", host);
    }
    
    freeaddrinfo(res);   // all done with this structure
    printf("Connection established!\n");

    return sockfd;
}

// get ip address, IPv4 or IPv6
void* get_ip_addr(struct sockaddr* sa)
{
    if (sa->sa_family == AF_INET) {
        return &((struct sockaddr_in*)sa)->sin_addr;
    }
    return &((struct sockaddr_in6*)sa)->sin6_addr;
}

void epoll_add(int epfd, int fd)
{
    struct epoll_event events;
    events.events = EPOLLIN;
    events.data.fd = fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &events);
}

void epoll_del(int epfd, int fd)
{
    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
}
