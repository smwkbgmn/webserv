#ifndef SERVER_HPP
#define SERVER_HPP

#include "ASocket.hpp"
// #include "Client.hpp"
#include "structure.hpp"
// typedef struct kevent kevent;

#define MAX_EVENTS 10
class Client;

class Server : ASocket {
 public:
  vec_config_t  conf;

  Server(void);
  Server(char * );
  ~Server(void);

  // vec_config_t  locationConfs;


  const vec_config_t &config(void) const { return conf; }

  void change_events(uintptr_t, int16_t, uint16_t, uint32_t, intptr_t, void *);
  void connect_sever( void );
  void ServerPreset();

  int eventOccure();
  bool errorcheck(struct kevent &);

  bool handleReadEvent(struct kevent *, int, std::map<int, std::stringstream> &,
                       Client &, osstream_t&, size_t&, size_t& );
  void handleWriteEvent(struct kevent *, std::map<int, std::stringstream> &);
  struct kevent &getEventList(int);


  
 private:
  int kq;

  // config_t serverConf;
  bool dataReceived;
  std::vector<struct kevent> server_list;
  struct kevent *occur_event;
  struct timespec timeout;


  
  char client_event[8];
  char server_event[8];
};

#include "Client.hpp"

#endif