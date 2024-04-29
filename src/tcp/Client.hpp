#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ASocket.hpp"
#include "Server.hpp"
#include <sstream>
#include <map>
#include <string>

#include <cstring>

// #define BuffSize 1024
#define BuffSize 3000
class Server;

class Client {
private:
    Server& srv;
    int client_socket;
    osstream_t oss;  
    osstream_t  response;

    ssize_t  byte_read;

public:


    Client(Server& server);
    ~Client();

    bool sendData();
    void disconnect_client(int client_fd);
    void processClientRequest( Client &);

    const char* buffer() const ;
    const std::string getBufferContents() const;
    // bool isRequestComplete(const std::string& request);
    bool isRequestComplete( const osstream_t& );

    const Server& getServer() const;
    const int& getSocket() const;

    void setSocket(const int& );
    void setServer(const Server& );
};

#endif // CLIENT_HPP
