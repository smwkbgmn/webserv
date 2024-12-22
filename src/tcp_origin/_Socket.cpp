#include "_Socket.hpp"

// Socket::Socket() : server_socket(-1) {}

 Socket::Socket() {}
Socket::~Socket() {
//     for (size_t i = 0; i < server_list.size(); ++i)
//         close(server_list[i]);
}

void Socket::socketOpen(int& server_socket) {
    
    server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == -1)
        throw err_t("Failed to create socket");
    server_list.push_back(server_socket);
}

void Socket::setAddr(int port,sockaddr_t & addr) {
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);
}

void Socket::preSet(int& server_socket, sockaddr_t& addr) {
    if (bind(server_socket, reinterpret_cast<const struct sockaddr*>(&addr), sizeof(addr)) == ERROR)
    {
        close(server_socket);
        throw err_t("Failed to bind");
    }
    if (listen(server_socket, 32) == ERROR)
    {
        close(server_socket);
        throw err_t("Failed to listen");
    }
}

void Socket::setNonBlocking(int fd) {
    if (fcntl(fd, F_SETFL, O_NONBLOCK) == -1) {
        close(fd);
        throw err_t("Failed to set non-blocking mode");
    }
}

void Socket::openSocket(int port, int &server_socket) {
    sockaddr_t addr;
    
    socketOpen(server_socket);
    setAddr(port,addr);

    // ////////// FOR TEST //////////

	// int optval = 1;
    // setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval,
    //            sizeof(optval));

    preSet(server_socket,addr);
    setNonBlocking(server_socket);
    
    std::clog<<" openport is  "<<port<<std::endl;
    // log::print("TCP\t: Listening on port 8080\n");
	// std::clog << "listening on port " << port << "\n";
}
