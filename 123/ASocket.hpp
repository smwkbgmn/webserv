#ifndef TCP_HPP
#define TCP_HPP

#include <fcntl.h>
#include <exception>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <fstream>
#include <unistd.h>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <iostream>
#include "error.hpp"
#include "log.hpp"
#include "structure.hpp"

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
//   std::vector<Server> sever_list;

  ASocket(void);
  ASocket(int);
  virtual ~ASocket(void) = 0;
  void setAddr(void);

  void setNonBlocking(void);
};


#endif