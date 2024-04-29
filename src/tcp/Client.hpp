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
    Server&		srv;
    int			client_socket;

    // osstream_t	oss;  
	std::stringstream	oss;  
    osstream_t	response;

    bool		header_done;
    ssize_t		body_size;
    ssize_t		body_read;

public:


    Client(Server& server);
    ~Client();

    bool sendData();
    void disconnect_client(int client_fd);
    void processClientRequest( Client &);

    const char* buffer() const;
    const std::string getBufferContents() const;
    // bool isRequestComplete(const std::string& request);
    bool    isHeaderDone( const char*, ssize_t& );
    bool isRequestComplete( const char*, const size_t& );

    const Server& getServer() const;
    const int& getSocket() const;

    void setSocket(const int& );
    void setServer(const Server& );
};

#endif // CLIENT_HPP
