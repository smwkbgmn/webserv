#include "Server.hpp"

/* INSTANCIATE */
Server::Server(const port_t& port):
Socket() {

	_open(port);
	setNonblock();

	std::cout << "Server " + std::to_string(sock()) + " has created on port " + std::to_string(port) + "\n";
}

Server::Server(Server&& target) noexcept:
Socket(std::move(target)), _conf(std::move(target._conf)) {}

Server::~Server() {}

void Server::_open(const port_t& port) {
	_openSetAddr(port);

	if (bind(sock(), reinterpret_cast<const sockaddr_t*>(&addr), addr_len) == ERROR) {
		throwSysErr("bind");
	}
	
	if (listen(sock(), MAX_CLIENT) == ERROR) {
		throwSysErr("listen");
	}
}

void Server::_openSetAddr(const int& port) {
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
}

/* OPERATOR */
Server& Server::operator=(Server&& target) noexcept {
	if (this != &target) {}
	
	return *this;
}

/* ACCESS */
const vec<config_t>& Server::config() const {
	return _conf;
}

/* METHOD */
void Server::configAdd(const config_t& conf) { 
	_conf.push_back(std::move(conf));
}