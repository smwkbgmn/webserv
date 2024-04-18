#include "Server.hpp"

#include "HTTP.hpp"

void printBuffer(const char *buff);
Server::Server(void) : ASocket(), server_list(8) {
  conf.push_back( config_t() );
}

Server::~Server(void) { close(server_socket); }

void Server::connect_sever() {
  int newEvent;

  std::map<int, std::string> findClient;
  char buffcopy[max];
  this->openSocket();

  ServerPreset();

  Client client(*this);
  while (1) {
    newEvent = eventOccure();

    for (int i = 0; i < newEvent; i++) {
      cur_event = &getEventList(i);

      errorcheck(*cur_event);

      if (cur_event->filter == EVFILT_READ) {
        if (!handleReadEvent(cur_event, server_socket, findClient, client)) {
          std::cout << "if its not working" << std::endl;
          continue;
        }
      } else if (cur_event->filter == EVFILT_WRITE) {
        std::string tempString = client.getBufferContents();
        std::strncpy(buffcopy, tempString.c_str(), max - 1);
        buffcopy[max - 1] = '\0';

        printBuffer(buffcopy);
        // HTTP::transaction(Request(*this, buff));
        // handleWriteEvent(cur_event, findClient);
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

bool Server::handleReadEvent(struct kevent *cur_event, int server_socket,
                             std::map<int, std::string> &findClient,
                             Client &client) {
  if (cur_event->ident == static_cast<uintptr_t>(server_socket)) {
    clog( "Wating for request..." );
    client_socket = accept(server_socket, NULL, NULL);
    client.setSocket(client_socket);
    if (client_socket == -1) {
      return false;
    }
    this->setNonBlocking(client_socket);
    findClient = client.getClients();
    change_events(client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);

    findClient[client_socket] = "";
  } else if (findClient.find(cur_event->ident) != findClient.end()) {
    client.processClientRequest(cur_event->ident, findClient, *this);
    // change_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0,
    //   &timeout);
  }
  return true;
}

void Server::handleWriteEvent(struct kevent *cur_event,
                              std::map<int, std::string> &findClient) {
  // HTTP::transaction(Request(*this, buf));
  // std::cout << "Sent response to client: " << std::endl;
  std::map<int, std::string>::iterator it = findClient.find(cur_event->ident);
  if (it != findClient.end()) {
    const std::string &response = it->second;

    ssize_t bytes_written =
        send(cur_event->ident, response.c_str(), response.length(), 0);
    if (bytes_written > 0) {
      // std::cout << "Sent response to client: " << cur_event->ident << std::endl;
      change_events(cur_event->ident, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
      findClient.erase(cur_event->ident);
    } else {
      close(cur_event->ident);
      throw std::runtime_error("Failed to write");
    }
  }
}

void Server::errorcheck(struct kevent &event) {
  if (event.flags & EV_ERROR) {
    if (event.ident == static_cast<uintptr_t>(server_socket)) {
      close(server_socket);
      throw err_t("Server socket Error");
    } else {
      close(client_socket);
      throw err_t("client socket Error");
    }
  }
}
void Server::ServerPreset() {
  this->kq = kqueue();
  if (kq == -1) throw err_t("Fail to create kqueue");
  change_events(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
  // change_events(server_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0,
  // NULL);
  timeout.tv_sec = 5;
  timeout.tv_nsec = 0;
}

void Server::change_events(uintptr_t ident, int16_t filter, uint16_t flags,
                           uint32_t fflags, intptr_t data, void *udata) {
  struct kevent temp_event;

  EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
  kevent(kq, &temp_event, 1, NULL, 0, NULL);
}

struct kevent &Server::getEventList(int idx) { return server_list[idx]; }

void printBuffer(const char *buff) {
  if (buff == nullptr) {
    std::cout << "Buffer is null." << std::endl;
    return;
  }

  for (int i = 0; buff[i] != '\0'; ++i) {
    std::cout << buff[i];
  }
  std::cout << std::endl;  // 줄바꿈 추가
}