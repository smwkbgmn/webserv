#include "ASocket.hpp"

ASocket::ASocket(void) {}

ASocket::~ASocket(void) { close(this->server_socket); }

void ASocket::openSocket() {

    socketOpen();
    setAddr();
    // socket timeout 설정
    int optval = 1;
    setsockopt(this->server_socket, SOL_SOCKET, SO_REUSEADDR, &optval,
               sizeof(optval));
    preSet();
    setNonBlocking(this->server_socket);
    clog( "Listening on port 8080" );

}

void ASocket::setAddr() {
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);
}

void ASocket::setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        close(fd);
        throw err_t("Failed to change status");
    }
}

void ASocket::socketOpen(void) {
    this->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    this->client_socket = 0;
    if (this->server_socket == -1)
        throw err_t("fail to create socket");
}

void ASocket::preSet(void) {

    if (bind(this->server_socket, (const struct sockaddr *)&addr,
             sizeof(addr)) == ERROR)
        throw err_t("fail to bind");
    if (listen(this->server_socket, 10) == ERROR)
        throw err_t("fail to listening");
}