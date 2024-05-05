#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server),client_socket(-1), msg( FALSE ), body_size( 0 ), body_read( 0 ) {}

Client::~Client() {}



const Server& Client::getServer() const {
    return srv;
}

const int& Client::getSocket() const {
    return client_socket;
}

const char* Client::buffer() const {
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
    ssize_t byte = recv(client_socket, buf, BuffSize, 0);

    osstream_t stream;
    stream << "TCP\t: receiving done by " << byte;
    clog( stream.str() );

    if (byte < 0) {
        disconnect_client(client_socket);
        throw err_t("Server socket error on receive");
    }
    
    else if (byte == 0) {
        std::cout << "Client disconnected on file descriptor " << client_socket << std::endl;
        disconnect_client(client_socket);
    }
    
    else {
        client.oss.write( buf, byte );

        logging.fs << oss.str() << std::endl;

        if (isMsgDone( buf, byte ) && isBodyDone( byte ) ) {
            HTTP::transaction(*this, client.subprocs, response);
            
            // Consider write a Client reset method
            client.oss.str( "" );
            client.oss.clear();

            client.msg          = FALSE;
            client.body_size    = 0;
            client.body_read	= 0;

            client.subprocs.reset();

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
Client::isMsgDone( const char* buf, ssize_t& byte_read ) {
	if ( !msg ) {
		str_t data_read( buf );

		size_t pos_header_end = str_t( buf ).find( "\r\n\r\n" );
		if ( pos_header_end != str_t::npos ) {
			clog( "TCP\t: end of header has found" );
			msg         = TRUE;
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
	return msg;
}

bool Client::isBodyDone(const size_t& byte_read) {
	body_read += byte_read;

	osstream_t oss;
	oss << "TCP\t: body read by " << byte_read << " so far: " << body_read << " / " << body_size << std::endl;
	clog( oss.str() );
	
	return body_size == body_read;
}

