#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "utill.hpp"

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

	msg_buffer_t	in;
	msg_buffer_t	out;
    // osstream_t		out;

	Request*		rqst;
	Response*		rspn;

    process_t		subprocs;

public:
    Client(Server& server);
    ~Client();

    bool sendData();              
    void disconnect_client(int client_fd);
    void processClientRequest( Client &);

	const msg_buffer_t&	buffer() const;

    const std::string getBufferContents() const;
    bool recvMsg( const char*, ssize_t& );
    bool recvBody( const char*, const size_t& );

    const Server& getServer() const;
	const Server& server() const;
    const int& getSocket() const;

    void setSocket(const int& );
    void setServer(const Server& );
};

#endif // CLIENT_HPP
