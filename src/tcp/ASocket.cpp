#include "ASocket.hpp"

ASocket::ASocket() : server_socket(-1) {}

ASocket::~ASocket() {
    for (size_t i = 0; i < socket_list.size(); ++i)
        close(socket_list[i]);
}

void ASocket::socketOpen() {
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
        throw err_t("Failed to create socket");
    socket_list.push_back(server_socket);
}

void ASocket::setAddr() {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
}

void ASocket::preSet() {
    if (bind(server_socket, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)) == ERROR)
        throw err_t("Failed to bind");
    if (listen(server_socket, 10) == ERROR)
        throw err_t("Failed to listen");
}

void ASocket::setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        close(fd);
        throw err_t("Failed to set non-blocking mode");
    }
}

void ASocket::openSocket() {
    socketOpen();
    setAddr();
    preSet();
    setNonBlocking(server_socket);
    clog("Listening on port 8080");
}
