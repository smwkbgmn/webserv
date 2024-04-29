#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server),client_socket(-1), header_done( FALSE ), body_size( 0 ), body_read( 0 ) {}

Client::~Client() {}



const Server& Client::getServer() const {
    return srv;
}

const int& Client::getSocket() const {
    return client_socket;
}

const char* Client::buffer() const {
    // return oss.str().c_str();
	// char* testbuf = new char[oss.str().size()];
    // return static_cast<const char*>( memcpy( testbuf, oss.str().c_str(), oss.str().size()) );
	size_t	size;
	return dupStreamBuf( oss, size );
}

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

    clog( "TCP\t: receiving data" );
    ssize_t n = recv(client_socket, buf, BuffSize, 0);

    osstream_t stream;
    stream << "TCP\t: receiving done by " << n;
    clog( stream.str() );

    if (n < 0) {
        disconnect_client(client_socket);
        throw err_t("Server socket error on receive");
    }
    
    else if (n == 0) {
        std::cout << "Client disconnected on file descriptor " << client_socket << std::endl;
        disconnect_client(client_socket);
    }
    
    else {
        // buf[n] = '\0';
        // client.oss << buf;
        // client.oss.write( buf, n );
        client.oss.write( buf, n );

        logging.fs << oss.str() << std::endl;

        if (isHeaderDone( buf, n ) && isRequestComplete( buf, n ) ) {
            HTTP::transaction(*this, response);

			client.oss.str( "" );
			client.oss.clear();

			client.header_done  = FALSE;
			client.body_size    = 0;
			client.body_read	= 0;

            srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
        }
    }
}

bool Client::sendData() 
{
        std::string data = response.str(); 
        const char* buffer = data.c_str();  
        size_t length = data.size(); 

        clog( "TCP\t: sending" );
        // std::clog << response.str() << std::endl;
        logging.fs << response.str() << "\n" << std::endl;

        ssize_t bytesSent = send(client_socket, buffer, length, 0);  
        if (bytesSent < 0) {
            return false;
        }

        // response.flush();
        response.str( "" );
        response.clear();  

        return true;
}


bool
Client::isHeaderDone( const char* buf, ssize_t& byte_read ) {
	if ( !header_done ) {
		str_t data_read( buf );

		size_t pos_header_end = str_t( buf ).find( "\r\n\r\n" );
		if ( pos_header_end != str_t::npos ) {
			clog( "TCP\t: end of header has found" );
			header_done	= TRUE;
			body_read	= byte_read - pos_header_end - 4;
			byte_read	= 0;

			size_t pos_header_len = data_read.find( "Content-Length" );
			if ( pos_header_len != str_t::npos ) {
				clog( "TCP\t: content-length header has found" );
				isstream_t  iss( data_read.substr( pos_header_len, data_read.find( CRLF, pos_header_len ) ) ); 
				str_t       discard;

				std::getline( iss, discard, ':' );
				iss >> std::ws >> body_size;

			}
		}
	}
	return header_done;
}

// bool Client::isRequestComplete(const std::string& request) {
bool Client::isRequestComplete(const char* buf, const size_t& byte_read) {
	(void)buf;

	body_read += byte_read;

	osstream_t oss;

	oss << "TCP\t: body read by " << byte_read << " so far: " << body_read << " / " << body_size << std::endl;
	clog( oss.str() );
	
	return body_size == body_read;
	
    

    // size_t headerEnd = request.find("\r\n\r\n");
    // if (headerEnd == std::string::npos) {
    //     return false;
    // }

    // size_t pos = request.find("Content-Length:");
    // if (pos != std::string::npos) {
    //     size_t start = pos + 15; 
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

