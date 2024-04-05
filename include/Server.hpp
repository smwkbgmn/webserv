#ifndef SERVER_HPP
#define SERVER_HPP

#include "ASocket.hpp"
#include "Client.hpp"

#define MAX_EVENTS 10

class Server : ASocket {
 public:
  Server(void);
  Server(char *);
  ~Server(void);


  void change_events( uintptr_t, int16_t ,
        uint16_t , uint32_t , intptr_t , void *); 
  void connect_sever();

  void disconnect_client(int  );
  struct kevent&  getEventList(int );
private:
  int kq;
  std::map<int, std::string> clients;
  std::vector<struct kevent> change_list;
  std::vector<struct kevent> server_list;
  struct kevent* cur_event;
  // struct kevent event_list[MAX_EVENTS];
};

#endif