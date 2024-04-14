#ifndef SERVER_HPP
#define SERVER_HPP

#include "ASocket.hpp"
#include "log.hpp"
// typedef struct kevent kevent;

#define MAX_EVENTS 10
class Client;

class Server : ASocket {
 public:
  vec_config_t  conf;

  Server(void);
  Server(char *);
  ~Server(void);

  const vec_config_t &config(void) const { return conf; }

  void change_events(uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void *);
  void connect_sever();
  void ServerPreset();

  int eventOccure();
  void errorcheck(struct kevent &);
  bool handleReadEvent(struct kevent *, int, std::map<int, std::string> &,
                       Client &);
  void handleWriteEvent(struct kevent *cur_event,
                        std::map<int, std::string> &findClient);
  struct kevent &getEventList(int);

 private:
  int kq;

  // int newEvent;

  std::vector<struct kevent> server_list;
  struct kevent *cur_event;
  struct timespec timeout;
};

#include "Client.hpp"

#endif