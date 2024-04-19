#ifndef TCP_HPP
#define TCP_HPP

#include "error.hpp"
#include "log.hpp"
#include "structure.hpp"
#include <cstring>
#include <exception>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <stdio.h>
#include <string>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

#define LOOP 1

/*
        [TCP functions using UNIX]
        socketpair
        htons
        htonl
        nthos
        ntohl

        select / poll / epoll / kqueue
        socket
        accept
        listen

        send
        recv

        bind
        connect

        getaddrinfo
        freeaddrinfo

        setsockpot
        getsockname
        getprotobyname
*/

class ASocket {

  public:
    struct sockaddr_in addr;
    socklen_t addrSize;
    socket_t server_socket;
    socket_t client_socket;

    std::vector<socket_t> socket_list;

    ASocket(void);
    ASocket(int);
    virtual ~ASocket(void);
    void setAddr(void);

    void setNonBlocking(int);
    void socketOpen(void);
    void preSet();

    void openSocket(void);
};

#endif