#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server),client_socket(-1), action(NULL) {}

Client::~Client() { if ( action ) delete action; }



const Server& Client::getServer() const {
    return srv;
}

const Server& Client::server() const { return srv; }

const int& Client::getSocket() const {
    return client_socket;
}

const msg_buffer_t& Client::buffer() const {
	return in;
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
    ssize_t byte = recv(client_socket, buf, in.next_read, 0);

    osstream_t stream;
    stream << "TCP\t: receiving done by " << byte;
    log( stream.str() );

    if ( byte > 0 && Transaction::recvMsg( in, buf, byte ) ) {
		try {
			if ( !action )
				action = new Transaction( *this );

			if ( Transaction::recvBody( in, subprocs, buf, byte ) ) {
				logging.fs << in.msg.str() << std::endl;
				logging.fs << in.body.str() << std::endl;

				if ( !subprocs.pid ) action->act();
				else action->actCGI();

				in.reset();
				subprocs.reset();

				srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
			}
		}

		catch ( errstat_t& err ) {
			log( "HTTP\t: transaction: " + str_t( err.what() ) );

			out.msg.str( "" );
			out.body.str( "" );

			Transaction::build( Response( client, err.code ), out );

			in.reset();
			subprocs.reset();
			srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
		}

		catch ( err_t& err ) {
			log( "HTTP\t: Request: " + str_t( err.what() ) );

			Transaction::build( Response( client, 400 ), out );

			in.reset();
			srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
		}
    }

    else if (byte == 0) {
        std::cout << "Client disconnected on file descriptor " << client_socket << std::endl;
        disconnect_client(client_socket);
    }
    
    else {
        disconnect_client(client_socket);
        throw err_t("Server socket error on receive");
    }
    
}

bool Client::sendData() 
{
	size_t length = out.msg.str().size(); 

	log( "TCP\t: sending\n" );
	logging.fs << out.msg.str() << "\n" << std::endl;

	ssize_t bytesSent = send(client_socket, out.msg.str().c_str(), length, 0);  
	if (bytesSent < 0) return false;

	//////////////////////////////////////////////////////////////////////

	if ( out.body.str().size() ) {
		length = out.body.str().size(); 

		logging.fs << out.body.str() << "\n" << std::endl;

		bytesSent = send(client_socket, out.body.str().c_str(), length, 0);  
		if (bytesSent < 0) return false;

	}
	
	out.reset();

	if ( action ) { delete action; action = NULL; }

	return true;
}

/* STRUCT */
msg_buffer_s::msg_buffer_s( void ) { reset(); }

void
msg_buffer_s::reset( void ) {
	msg.str( "" );
	msg.clear();

	msg_done	= FALSE;
	msg_read	= 0;

	body.str( "" );
	body.clear();

	body_size	= 0;
	body_read	= 0;

	chunk		= FALSE;
	chunk_size	= 0;

	incomplete	= FALSE;
	next_read	= SIZE_BUFF;
}

process_s::process_s( void ) { reset();	}

void
process_s::reset( void ) {
	pid			= NONE;
	stat		= NONE;
	fd[R]		= NONE;
	fd[W]		= NONE;

	argv.clear();
	env.clear();
}