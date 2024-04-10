#ifndef SERVER_HPP
#define SERVER_HPP

#include "ASocket.hpp"
#include "Client.hpp"
// typedef struct kevent kevent;

#define MAX_EVENTS 10

class Server : ASocket {
  public:
    Server(void);
    Server(char *);
    ~Server(void);

    void change_events(uintptr_t, int16_t, uint16_t, uint32_t, intptr_t,
                       void *);
    void connect_sever();
    void preSet();

    size_t eventOccure();
    void errorcheck(struct kevent &);
    bool handleReadEvent(struct kevent *cur_event, int server_socket,
                         std::map<int, std::string> &findClient);
    void handleWriteEvent(struct kevent *cur_event,
                          std::map<int, std::string> &findClient);
    struct kevent &getEventList(int);

  private:
    int kq;

    std::vector<struct kevent> server_list;
    struct kevent *cur_event;
    struct timespec timeout;
};

#endif