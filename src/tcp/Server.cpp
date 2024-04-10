#include "Server.hpp"
#include "HTTP.hpp"

Server::Server(void) : ASocket(), server_list(8) {}

Server::~Server(void) { close(server_socket); }

void Server::connect_sever() {
    size_t newEvent;

    std::map<int, std::string> findClient;

    this->openSocket();
    Client client(*this);

    while (1) {
        newEvent = eventOccure();
        for (int i = 0; i < newEvent; i++) {
            cur_event = &getEventList(i);

            errorcheck(*cur_event);

            if (cur_event->filter == EVFILT_READ) {
                if (!handleReadEvent(cur_event, server_socket, findClient))
                    continue;
            } else if (cur_event->filter == EVFILT_WRITE) {
                handleWriteEvent(cur_event, findClient);
            }
        }
    }
}

size_t Server::eventOccure() {
    int occure;
    occure = kevent(this->kq, NULL, 0, &server_list[0], 8, &timeout);
    if (occure == -1)
        throw err_t("Fail to Make event");
    return occure;
}

bool Server::handleReadEvent(struct kevent *cur_event, int server_socket,
                             std::map<int, std::string> &findClient) {
    Client client(*this);
    int client_socket;

    if (cur_event->ident == static_cast<uintptr_t>(server_socket)) {
        client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1) {
            return false;
        }
        this->setNonBlocking(client_socket);
        findClient = client.getClients();
        change_events(client_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0,
                      NULL);
        findClient[client_socket] = "";
    } else if (findClient.find(cur_event->ident) != findClient.end()) {
        client.processClientRequest(cur_event->ident, findClient);
    }
    return true;
}

void Server::handleWriteEvent(struct kevent *cur_event,
                              std::map<int, std::string> &findClient) {

    std::map<int, std::string>::iterator it = findClient.find(cur_event->ident);
    if (it != findClient.end()) {
        const std::string &response = it->second;

        ssize_t bytes_written =
            send(cur_event->ident, response.c_str(), response.length(), 0);
        if (bytes_written > 0) {
            std::cout << "Sent response to client: " << cur_event->ident
                      << std::endl;
            change_events(cur_event->ident, EVFILT_WRITE, EV_DELETE, 0, 0,
                          NULL);
            findClient.erase(cur_event->ident);
        } else {
            close(cur_event->ident);
            throw std::runtime_error("Failed to write");
        }
    }
}

void Server::errorcheck(struct kevent &event) {
    if (cur_event->flags & EV_ERROR) {
        if (cur_event->ident == static_cast<uintptr_t>(server_socket)) {
            close(server_socket);
            throw err_t("Server socket Error");
        } else {
            close(client_socket);
            throw err_t("client socket Error");
        }
    }
}
void Server::preSet() {
    this->kq = kqueue();
    if (kq == -1)
        throw err_t("Fail to create kqueue");
    change_events(server_socket, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, NULL);
    timeout.tv_sec = 30;
    timeout.tv_nsec = 0;
}

void Server::change_events(uintptr_t ident, int16_t filter, uint16_t flags,
                           uint32_t fflags, intptr_t data, void *udata) {
    struct kevent temp_event;

    EV_SET(&temp_event, ident, filter, flags, fflags, data, udata);
    kevent(kq, &temp_event, 1, NULL, 0, NULL);
}

struct kevent &Server::getEventList(int idx) { return server_list[idx]; }