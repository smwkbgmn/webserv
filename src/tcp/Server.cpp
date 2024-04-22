#include "Server.hpp"
#include "HTTP.hpp"

// vec_config_t  confs;

void printBuffer(const char *buff);
Server::Server(void) : ASocket(), server_list(8), client_event("client"), server_event("serv")				
																 {
  this->kq = kqueue();
  if (kq == -1) throw err_t("Fail to create kqueue");
  timeout.tv_sec = 5;
  timeout.tv_nsec = 0;
  
  conf.push_back( config_t() );
 
}

Server::~Server(void) {   
  for (unsigned int i = 0; i < socket_list.size(); i++)
		close(socket_list[i]); 
  close(kq);  
  }

void Server::connect_sever( void ) {
  int newEvent;

//  std::vector<config_t> myServerConfigs;
//   std::vector<config_t>::iterator start = myServerConfigs.begin();
//   std::vector<config_t>::iterator finish = myServerConfigs.end();

  // for (int i =0; i <10; i++)
  // {
    ServerPreset();
  // }
  // for (; start != finish; start++) {
  //   ServerPreset();
  // }

  osstream_t  oss;
  size_t      bodysize = 0;
  size_t      total = 0;
  std::map<int, std::stringstream> findClient;
  Client client(*this, findClient);
  // client.setServerConfigs(myServerConfigs);
  while (1) {
    newEvent = eventOccure();
    
    for (int i = 0; i < newEvent; i++) {
      occur_event = &getEventList(i);

      errorcheck(*occur_event);

        if (occur_event->filter == EVFILT_READ) {
          if (!handleReadEvent(occur_event, server_socket, findClient, client, oss, bodysize, total)) {
            // transasction > respond msg compl > oss( resonse msg )
            continue;
          }
        } else if (occur_event->filter == EVFILT_WRITE) {

          // clog( "respond to client with message below" );
          // std::clog << oss.str() << std::endl;

          if ( send( client_socket, oss.str().c_str(), oss.str().size(), 0 ) == ERROR )
            throw err_t( "fail to send" );

          oss.flush();
          oss.clear();
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
                             std::map<int, std::stringstream> &findClient,
                             Client &client, osstream_t& oss, size_t& bodysize, size_t& total) {
  char *check_type =  static_cast<char*>(occur_event->udata);              
 if (std::strcmp(check_type, "serv") == 0) {
    client_socket = accept(server_socket, NULL, NULL);
    client.setSocket(client_socket);
    if (client_socket == -1) {
      return false;
    }
    this->setNonBlocking(client_socket);
    // findClient = client.getClients();
    change_events(client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, &client_event);

    // findClient[client_socket] = "";
  } else if(std::strcmp(check_type, "client") == 0){
    client.processClientRequest(occur_event->ident, findClient, oss, bodysize, total);
  }
  return true;
}

void Server::handleWriteEvent(struct kevent *occur_event,
                              std::map<int, std::stringstream> &findClient) {
  std::cout << "Sent response to client: " << std::endl;
  std::map<int, std::stringstream>::iterator it = findClient.find(occur_event->ident);
  if (it != findClient.end()) {
    const std::string &response = it->second.str();

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
