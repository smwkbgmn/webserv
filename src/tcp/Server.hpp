#ifndef SERVER_HPP
#define SERVER_HPP

#include "ASocket.hpp"
#include "Client.hpp"
#include "structure.hpp"
// typedef struct kevent kevent;

#define MAX_EVENTS 10
class Client;

class Server : ASocket {
 public:
  vec_config_t  conf;

  Server(void);
  Server(char *);
  ~Server(void);

  // vec_config_t  locationConfs;

  const config_t &config(void) const { return serverConf; }

  void change_events(uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void *);
  void connect_sever(std::vector<config_t> &);
  void ServerPreset();

  int eventOccure();
  void errorcheck(struct kevent &);

  bool handleReadEvent(struct kevent *, int, std::map<int, std::string> &,
                       Client &);
  void handleWriteEvent(struct kevent *, std::map<int, std::string> &);
  struct kevent &getEventList(int);

  const config_t &servConf() const { return serverConf; }

 private:
  int kq;

  config_t serverConf;
  bool dataReceived;
  std::vector<struct kevent> server_list;
  struct kevent *occur_event;
  struct timespec timeout;

  char client_event[8];
  char server_event[8];
};

#include "Client.hpp"

#endif