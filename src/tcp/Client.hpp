#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "utill.hpp"

#include "ASocket.hpp"
#include "Server.hpp"

#include <sstream>
#include <map>
#include <string>

#include <cstring>

#define SIZE_BUFF 1024
#define SIZE_CRLF 2
#define SIZE_CHUNK_HEAD 3

#include "Transaction.hpp"

class Client {
private:
    Server&		srv;
    int			client_socket;

    // osstream_t		out;

	Transaction*	action;

	// Request*		rqst;
	// Response*		rspn;


	bool recvBodyChunk( const char*, const size_t& );
	bool recvBodyPlain( const char*, const size_t& );
	

public:
	msg_buffer_t	in;
	msg_buffer_t	out;

    process_t		subprocs;

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
