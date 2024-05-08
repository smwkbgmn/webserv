#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server),client_socket(-1) {}

Client::~Client() {}



const Server& Client::getServer() const {
    return srv;
}

const Server& Client::server() const { return srv; }

const int& Client::getSocket() const {
    return client_socket;
}

const char* Client::buffer() const {
	size_t	size;
	return dupStreamBuf( msg.ss, size );
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
    char buf[SIZE_BUF];

    log( "TCP\t: receiving data" );
    ssize_t byte = recv(client_socket, buf, SIZE_BUF, 0);

    osstream_t stream;
    stream << "TCP\t: receiving done by " << byte;
    log( stream.str() );

    if (byte < 0) {
        disconnect_client(client_socket);
        throw err_t("Server socket error on receive");
    }
    
    else if (byte == 0) {
        std::cout << "Client disconnected on file descriptor " << client_socket << std::endl;
        disconnect_client(client_socket);
    }
    
    else {
        msg.ss.write( buf, byte );

        logging.fs << msg.ss.str() << std::endl;

        if (isMsgDone( buf, byte ) && isBodyDone( byte ) ) {
            HTTP::transaction( *this, client.subprocs, response );
            // if ( subprocs.pid != 0 ) {
                // Regist event
            // }

            // if ( subprocs is done )
            // _read ( from pipe )
            // _build
            // send
            
            // Consider write a Client reset method

			msg.reset();
        	subprocs.reset();

            srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
        }
    }
}

bool Client::sendData() 
{
        std::string data = response.str(); 
        const char* buffer = data.c_str();  
        size_t length = data.size(); 

        log( "TCP\t: sending\n" );
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
	if ( !msg.header_done ) {
		str_t data_read( buf );

		size_t pos_header_end = str_t( buf ).find( "\r\n\r\n" );
		if ( pos_header_end != str_t::npos ) {
			// log( "TCP\t: end of header has found" );
			msg.header_done	= TRUE;
			msg.body_read	= byte_read - pos_header_end - 4;
			byte_read		= 0;

			size_t pos_header_len = data_read.find( "Content-Length" );
			if ( pos_header_len != str_t::npos ) {
				// log( "TCP\t: content-length header has found" );
				isstream_t  iss( data_read.substr( pos_header_len, data_read.find( CRLF, pos_header_len ) ) ); 
				str_t       discard;

				std::getline( iss, discard, ':' );
				iss >> std::ws >> msg.body_size;
			}
		}
	}
	return msg.header_done;
}

bool Client::isBodyDone(const size_t& byte_read) {
	msg.body_read += byte_read;

    if ( byte_read ) {
        osstream_t oss;
        oss << "TCP\t: body read by " << byte_read << " so far: " << msg.body_read << " / " << msg.body_size << std::endl;
        log( oss.str() );
    }
	
	return msg.body_size == msg.body_read;
}

/* STRUCT */
msg_buffer_s::msg_buffer_s( void ) { reset(); }

void
msg_buffer_s::reset( void ) {
	ss.str( "" );
	ss.clear();

	body_size	= 0;
	body_read	= 0;
	
	header_done	= FALSE;
}

process_s::process_s( void ) { reset();	}

void
process_s::reset( void ) {
	pid			= NONE;
	stat		= NONE;
	fd[R]		= NONE;
	fd[W]		= NONE;	
}