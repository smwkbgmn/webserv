#include "Socket.hpp"

/* ACCESS */
const fd_t& Socket::sock() const { return _sock; }

/* INSTANTIATE */

/* Open Sever endpoint */
Socket::Socket() {
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == ERROR) { throwSysErr("socket"); }

	memset(&addr, NONE, sizeof(sockaddr_in_t));
	addr_len = sizeof(addr);
}

/* Accept Client connection */
Socket::Socket(const fd_t& sock_srv) {
	_sock = accept(sock_srv, reinterpret_cast<sockaddr_t*>(&addr), &addr_len);
	if (_sock == ERROR) { throwSysErr("accept"); }

	log::print("Client " + std::to_string(_sock) + " has connected to Server " + std::to_string(sock_srv));
}

Socket::Socket(const Socket&& source) noexcept:
	_sock(source._sock) {
	std::cout << "socket move constructor has called\n";

	/* Prevent source object from closing fd in destructor */
	const_cast<Socket&>(source)._sock = -1;
}

Socket::~Socket() {
	if (_sock) {
		close(_sock);
	}
}

/* OPERATOR */
Socket&	Socket::operator=(const Socket&& source) noexcept {
	if (this != &source) {
		// Do move things
	}
	return *this;
}

/* METHOD */
void Socket::setNonblock() const {
	int flags = fcntl(_sock, F_GETFL, 0);
	fcntl(_sock, F_SETFL, flags | O_NONBLOCK);
}
