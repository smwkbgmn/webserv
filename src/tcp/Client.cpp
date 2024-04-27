#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server),client_socket(-1) {}

Client::~Client() {}



const Server& Client::getServer() const {
    return srv;
}

const int& Client::getSocket() const {
    return client_socket;
}

const char* Client::buffer() const {
    return oss.str().c_str();
};

void Client::setSocket(const int& socket ){
    client_socket = socket;
}
void Client::setServer(const Server& serv){
    srv = serv;
}


void Client::disconnect_client(int client_fd) {
    std::cout << "Client disconnected: " << client_fd << std::endl;
    close(client_fd);
}



void Client::processClientRequest(Client& client) {
    char buf[BuffSize];
    int n = recv(client_socket, buf, BuffSize-1, 0);
    if (n < 0) {
        disconnect_client(client_socket);
        throw err_t("Server socket error on receive");
    } else if (n == 0) {
        std::cout << "Client disconnected on file descriptor " << client_socket << std::endl;
        disconnect_client(client_socket);
    } else {
        buf[n] = '\0';
        client.oss << buf;
        if (isRequestComplete(oss.str())) {
            HTTP::transaction(*this, oss);
            srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
        }
    }
}

bool Client::sendData() 
{
        std::string data = oss.str(); 
        const char* buffer = data.c_str();  
        size_t length = data.size(); 

        ssize_t bytesSent = send(client_socket, buffer, length, 0);  
        if (bytesSent < 0) {
            return false;
        }

        oss.str("");  
        oss.clear();  
        return true;
}


bool Client::isRequestComplete(const std::string& request) {
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return false;
    }

    size_t pos = request.find("Content-Length:");
    if (pos != std::string::npos) {
        size_t start = pos + 15; 
        size_t end = request.find("\r\n", start);
        if (end == std::string::npos) {
            return false;
        }
        int contentLength = std::stoi(request.substr(start, end - start));
        size_t contentStart = headerEnd + 4;
        return request.length() >= contentStart + contentLength;
    }

    pos = request.find("Transfer-Encoding: chunked");
    if (pos != std::string::npos) {
        if (request.find("0\r\n\r\n", headerEnd) != std::string::npos) {
            return true;
        }
        return false;
    }

    return true;
}

