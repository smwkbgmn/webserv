#include "Socket.hpp"

/* INSTANTIATE */
/* Open Sever endpoint */
Socket::Socket() { 
	_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (_sock == ERROR) {
		throwSysErr("socket");
	}

	memset(&addr, NONE, sizeof(sockaddr_in_t));
	addr_len = sizeof(addr);
}

/* Accept Client connection */
Socket::Socket(const fd_t& sock_srv) {
	_sock = accept(sock_srv, reinterpret_cast<sockaddr_t*>(&addr), &addr_len);
	if (_sock == ERROR) {
		throwSysErr("accept");
	}
}

Socket::Socket(const Socket&& source) noexcept:
addr(std::move(source.addr)), addr_len(source.addr_len), _sock(source._sock) {
	
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
	if (this != &source) {}
	
	return *this;
}

/* ACCESS */
const fd_t& Socket::sock() const { return _sock; }

/* METHOD */
void Socket::setNonblock() const {
	int flags = fcntl(_sock, F_GETFL, 0);
	fcntl(_sock, F_SETFL, flags | O_NONBLOCK);
}
