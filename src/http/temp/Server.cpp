#include "Server.hpp"
#include "HTTP.hpp"

// vec_config_t  confs;

void printBuffer(const char *buff);
Server::Server(void) : ASocket(), server_list(8), dataReceived(false),server_event("serv"),				
																client_event("client") {
  this->kq = kqueue();
  if (kq == -1) throw err_t("Fail to create kqueue");
  timeout.tv_sec = 5;
  timeout.tv_nsec = 0;
 
}

Server::~Server(void) {   
  for (unsigned int i = 0; i < socket_list.size(); i++)
		close(socket_list[i]); 
  close(kq);  
  }

void Server::connect_sever(std::vector<config_t> &myServerConfigs) {
  int newEvent;

  std::vector<config_t>::iterator start = myServerConfigs.begin();
  std::vector<config_t>::iterator finish = myServerConfigs.end();
  std::map<int, std::string> findClient;

  for (; start != finish; start++) {
    ServerPreset();
  }

  Client client(*this);
  client.setServerConfigs(myServerConfigs);
  while (1) {
    newEvent = eventOccure();
    
    for (int i = 0; i < newEvent; i++) {
      occur_event = &getEventList(i);

      errorcheck(*occur_event);
      if (occur_event->filter == EVFILT_TIMER) {
        if (!dataReceived) {
          throw std::runtime_error("Data was not received within 30 seconds");
        }
        dataReceived = false;
        if (occur_event->filter == EVFILT_READ) {
          if (!handleReadEvent(occur_event, server_socket, findClient, client)) {
            dataReceived = true;
            continue;
          }
        } else if (occur_event->filter == EVFILT_WRITE) {
          
        }
        } else if (occur_event->filter == EVFILT_PROC) {
          //request read 하고 cgi면 fork -> event 등록 -> execve
        }
      }
    }
  }


int Server::eventOccure() {
  int occure;
  occure = kevent(this->kq, NULL, 0, &server_list[0], 8, &timeout);

  if (occure == -1) throw err_t("Fail to Make event");
  return occure;
}

bool Server::handleReadEvent(struct kevent *occur_event, int server_socket,
                             std::map<int, std::string> &findClient,
                             Client &client) {
  char *check_type =  static_cast<char*>(occur_event->udata);              
 if (std::strcmp(check_type, "serv") == 0) {
    client_socket = accept(server_socket, NULL, NULL);
    client.setSocket(client_socket);
    if (client_socket == -1) {
      return false;
    }
    this->setNonBlocking(client_socket);
    findClient = client.getClients();
    change_events(client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client_event);

    findClient[client_socket] = "";
  } else if(std::strcmp(check_type, "client") == 0){
    client.processClientRequest(occur_event->ident, findClient, *this);
  }
  return true;
}

void Server::handleWriteEvent(struct kevent *occur_event,
                              std::map<int, std::string> &findClient) {
  std::cout << "Sent response to client: " << std::endl;
  std::map<int, std::string>::iterator it = findClient.find(occur_event->ident);
  if (it != findClient.end()) {
    const std::string &response = it->second;

    ssize_t bytes_written =
        send(occur_event->ident, response.c_str(), response.length(), 0);
    if (bytes_written > 0) {
      std::cout << "Sent response to client: " << occur_event->ident << std::endl;
      change_events(occur_event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
      findClient.erase(occur_event->ident);
    } else {
      close(occur_event->ident);
      throw std::runtime_error("Failed to write");
    }
  }
}

bool Server::errorcheck(struct kevent & kevent) {
    bool isEof = (kevent.flags & EV_EOF) && (kevent.filter != EVFILT_PROC);
    bool hasError = (kevent.flags & EV_ERROR);

    if (isEof || hasError) {
        close(kevent.ident);  
        return true; 
    }

    return false;  
}

void Server::ServerPreset() {
  this->openSocket();
  change_events(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &server_event);
}

void Server::change_events(uintptr_t ident, int16_t filter, uint16_t flags,
                           uint32_t fflags, intptr_t data, void *udata) {
  struct kevent temp_event;

  EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
  kevent(kq, &temp_event, 1, NULL, 0, NULL);
}

struct kevent &Server::getEventList(int idx) { return server_list[idx]; }
