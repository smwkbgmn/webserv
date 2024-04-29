#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server, std::map<int, std::stringstream>& buf ) : clients(buf), msg(NULL) ,srv(connect_server), byte_read(0) {}

Client::~Client() {}


void Client::disconnect_client(int client_fd) {
  std::cout << "client disconnected: " << client_fd << std::endl;
  close(client_fd);
}

void Client::processClientRequest(int fd,
                                  std::map<int, std::stringstream> &findClient,
                                  osstream_t& oss, size_t& bodysize, size_t& total) {
  // bool isChunked = false;

  // if (isChunked) {
  //     handleChunkedRequest(fd, findClient);
  // } else {

  handleRegularRequest(fd, findClient, oss, bodysize, total);
}

// void handledSend(const Client &client) {
//   (void)client;
//   // checking
//   // sending
//   // checking
// }

// void Client::handleChunkedRequest(int fd,
//                                   std::map<int, std::stringstream> &findClient) {
//   bool check_chunked = true;
//   std::string chunkData;
//   unsigned int chunkSize = 0;
//   std::string tempBuf;

//   while (true) {
//     if (check_chunked) {
//       char sizeBuf[10];
//       std::memset(sizeBuf, 0, sizeof(sizeBuf));
//       int bytesRead = read(fd, sizeBuf, sizeof(sizeBuf) - 2);
//       if (bytesRead <= 0) {
//         throw err_t("read error");
//       }
//       std::string sizeStr(sizeBuf);
//       std::size_t pos = sizeStr.find("\r\n");
//       if (pos != std::string::npos) {
//         sizeStr = sizeStr.substr(0, pos);
//         std::istringstream(sizeStr) >> std::hex >> chunkSize;
//         if (chunkSize == 0) break;
//         check_chunked = false;
//       }
//     } else {
//       char dataBuf[chunkSize + 3];
//       std::memset(dataBuf, 0, sizeof(dataBuf));
//       int bytesRead = read(fd, dataBuf, chunkSize + 2);
//       if (bytesRead <= 0) {
//         throw err_t("Server socket Error");
//       }
//       dataBuf[bytesRead - 2] = '\0';
//       chunkData = dataBuf;
//       findClient[fd] += chunkData;
//       check_chunked = true;
//     }
//   }

//   if (!chunkData.empty()) {
//     std::cout << "Received chunked data from " << fd << ": " << findClient[fd]
//               << std::endl;
//   } else {
//     std::cout << "No chunked data received or connection closed prematurely."
//               << std::endl;
//   }
// }

#include <cstring>

void Client::handleRegularRequest(int fd, std::map<int, std::stringstream> &findClient, osstream_t& oss, size_t& bodysize, size_t& total) {
    byte_read = 0;

    if ( msg ) {
      delete[] msg;
      msg = NULL;
    }

    clog( "handleRegularRequest - recv start" );
    byte_read = recv(fd, buf, SIZE_BUF, 0);
  
    osstream_t stream;
    stream << "handleRegularRequest - recv done read by " << byte_read;
    clog( stream.str() );

    // clog( "handleRegularRequest - recv data" );
    // for ( ssize_t idx = 0; idx < byte_read; ++idx ) std::clog << msg[idx];
    // std::clog << std::endl;

    if (byte_read < 0) {
      msg = NULL;
      std::cerr << "Client receive error on file descriptor " << fd << ": " << strerror(errno) << std::endl;
      disconnect_client(fd);
      throw err_t("Server socket error on receive");
    }
    
    else if (byte_read == 0) {
      msg = NULL;
      std::cout << "Client disconnected on file descriptor " << fd << std::endl;
      disconnect_client(fd);
    }
    
    else {
        msg = new char[byte_read];
        memcpy( msg, buf, byte_read );

        findClient[fd].write( msg, byte_read );
        
        // clog( "handleRegularRequest - copied msg\n" );
        // for ( ssize_t idx = 0; idx < byte_read; ++idx )
        //   std::clog << findClient[fd].str().at(idx);
        // std::clog << std::endl;

        // std::cout << "buff contetnts " << fd << " : " << findClient[fd] <<std::endl;
        // if (isRequestComplete(msg, n, bodysize, total)) {
        //   clog( "Have done Request msg" );
          HTTP::transaction( *this, oss );
          srv.change_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);

          (void)bodysize;
          (void)total;
          // bodysize = 0;
          // total = 0;
        // }
    }
}


bool Client::isRequestComplete(const char* data_receive, const size_t& n, size_t& bodysize, size_t& total) {
  str_t request( data_receive );
   size_t  begin = request.find( "Content-Length" );
  
  if ( begin != str_t::npos ) {
    size_t  end = request.find( CRLF, begin );
    if ( end != str_t::npos ) {
      // std::clog << "content length begin and end" << begin << ", " << end << std::endl;
      isstream_t  iss( request.substr( begin + 16, end ) );
      iss >> bodysize;
      // total += request.size();
      // clog( "isRequestComplete - the POST request (start line)" );
      // std::clog << request;
    }
    return total >= bodysize;
  }

  else {
    if ( bodysize ) {
      total += n;
      // clog( "isRequestComplete - the POST request" );
      // std::clog << data_receive << std::endl;
      std::clog << total << " / " << bodysize << std::endl;
    }
    return total >= bodysize;
  }


  
    // size_t headerEnd = request.find("\r\n\r\n");
    // if (headerEnd == std::string::npos) {
    //     return false;  
    // }

    // size_t pos = request.find("Content-Length:");
    // if (pos != std::string::npos) {
    //     size_t start = pos + 16;
    //     size_t end = request.find("\r\n", start);
    //     if (end == std::string::npos) {
    //         return false; 
    //     }
    //     int contentLength = std::stoi(request.substr(start, end - start));
    //     size_t contentStart = headerEnd + 4;
    //     return request.length() >= contentStart + contentLength;
    // }

    // pos = request.find("Transfer-Encoding: chunked");
    // if (pos != std::string::npos) {
       
    //     if (request.find("0\r\n\r\n", headerEnd) != std::string::npos) {
    //         return true;  
    //     }
    //     return false; 
    // }

    // return true; 
}

// void Client::processFullRequest(int fd, const std::string& request) {
//   (void)fd;
//     (void) request;
//     // std::cout << "Full request received from fd " << fd << ": " << request << std::endl;
    
// }

const std::map<int, std::stringstream> &Client::getClients() const { return clients; }
const Server &Client::getserver(void) const { return srv; };