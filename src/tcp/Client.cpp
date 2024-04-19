#include "Client.hpp"

#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server) {}

Client::~Client() {}


void Client::disconnect_client(int client_fd) {
  std::cout << "client disconnected: " << client_fd << std::endl;
  close(client_fd);
}

void Client::processClientRequest(int fd,
                                  std::map<int, std::string> &findClient,
                                  Server &server) {
  // bool isChunked = false;

  // if (isChunked) {
  //     handleChunkedRequest(fd, findClient);
  // } else {

  handleRegularRequest(fd, findClient);
  server.change_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0,
                       NULL);
  
  osstream_t oss;

  HTTP::transaction(*this);


  // }
}

void handledSend(const Client &client) {
  // checking
  // sending
  // checking
}

void Client::handleChunkedRequest(int fd,
                                  std::map<int, std::string> &findClient) {
  bool check_chunked = true;
  std::string chunkData;
  unsigned int chunkSize = 0;
  std::string tempBuf;

  while (true) {
    if (check_chunked) {
      char sizeBuf[10];
      std::memset(sizeBuf, 0, sizeof(sizeBuf));
      int bytesRead = read(fd, sizeBuf, sizeof(sizeBuf) - 2);
      if (bytesRead <= 0) {
        throw err_t("read error");
      }
      std::string sizeStr(sizeBuf);
      std::size_t pos = sizeStr.find("\r\n");
      if (pos != std::string::npos) {
        sizeStr = sizeStr.substr(0, pos);
        std::istringstream(sizeStr) >> std::hex >> chunkSize;
        if (chunkSize == 0) break;
        check_chunked = false;
      }
    } else {
      char dataBuf[chunkSize + 3];
      std::memset(dataBuf, 0, sizeof(dataBuf));
      int bytesRead = read(fd, dataBuf, chunkSize + 2);
      if (bytesRead <= 0) {
        throw err_t("Server socket Error");
      }
      dataBuf[bytesRead - 2] = '\0';
      chunkData = dataBuf;
      findClient[fd] += chunkData;
      check_chunked = true;
    }
  }

  if (!chunkData.empty()) {
    std::cout << "Received chunked data from " << fd << ": " << findClient[fd]
              << std::endl;
  } else {
    std::cout << "No chunked data received or connection closed prematurely."
              << std::endl;
  }
}

void Client::handleRegularRequest(int fd, std::map<int, std::string> &findClient) {
    const size_t bufferSize = sizeof(buf) - 1;
    int n = recv(fd, buf, bufferSize, 0);
    if (n < 0) {
        std::cerr << "Client receive error on file descriptor " << fd << ": " << strerror(errno) << std::endl;
        disconnect_client(fd);
        throw err_t("Server socket error on receive");
    } else if (n == 0) {
        std::cout << "Client disconnected on file descriptor " << fd << std::endl;
        disconnect_client(fd);
    } else {
        buf[n] = '\0';
        findClient[fd] += buf;

        if (isRequestComplete(findClient[fd])) {
            processFullRequest(fd, findClient[fd]);
            // findClient[fd].clear();
            
        }
    }
}


bool Client::isRequestComplete(const std::string& request) {
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return false;  
    }

    size_t pos = request.find("Content-Length:");
    if (pos != std::string::npos) {
        size_t start = pos + 16;
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

void Client::processFullRequest(int fd, const std::string& request) {
    
    std::cout << "Full request received from fd " << fd << ": " << request << std::endl;
    
}

const std::map<int, std::string> &Client::getClients() const { return clients; }
const Server &Client::getserver(void) const { return srv; };