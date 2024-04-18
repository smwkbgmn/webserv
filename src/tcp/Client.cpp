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
  bool readChunkSize = true;
  std::string chunkData;
  unsigned int chunkSize = 0;
  std::string tempBuf;

  while (true) {
    if (readChunkSize) {
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
        readChunkSize = false;
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
      readChunkSize = true;
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

void Client::handleRegularRequest(int fd,
                                  std::map<int, std::string> &findClient) {
  int n = recv(fd, buf, sizeof(buf) - 1, 0);
  if (n < 0) {
    std::cerr << "client recv error!" << std::endl;
    disconnect_client(fd);
    throw err_t("Server socket Error");
  } else if (n == 0) {
    disconnect_client(fd);
  } else {
    buf[n] = '\0';
    findClient[fd] += buf;
  }
}

const std::map<int, std::string> &Client::getClients() const { return clients; }
const Server &Client::getserver(void) const { return srv; };