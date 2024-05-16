#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server),client_socket(-1), rqst(NULL), rspn(NULL) {}

Client::~Client() { if ( rqst ) delete rqst; if ( rspn ) delete rspn; }



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

    if ( byte > 0 && recvMsg( buf, byte ) ) {
		try {
			if ( !rqst ) {
				rqst = new Request( *this );

				if ( rqst->line().method == POST ) {
					if ( rqst->header().transfer_encoding == TE_CHUNKED )
						in.chunk = TRUE;
					else
						in.body_size = rqst->header().content_length;
				}
			}

			// if ( HTTP::invokeCGI( *rqst, subprocs ) )
			// 	CGI::proceed( *rqst, subprocs, out.body );

			if ( rqst->header().transfer_encoding == TE_UNKNOWN )
				throw errstat_t( 501, err_msg[TE_NOT_IMPLEMENTED] );

			if ( !getInfo( rqst->line().uri, rqst->info ) ) {
				if ( errno == 2 ) throw errstat_t( 404, err_msg[SOURCE_NOT_FOUND] );
				else throw errstat_t( 500 );
			}

			if ( recvBody( buf, byte ) ) {
				logging.fs << in.msg.str() << std::endl;
				logging.fs << in.body.str() << std::endl;

				// HTTP::transaction( *this, client.subprocs, out );

				if ( !subprocs.pid ) {
					rspn = new Response( *rqst );
					HTTP::build( *rspn, out );
				}

				else {
					if ( !in.chunk )
						CGI::writeTo ( subprocs, in.body.str().c_str(), in.body.str().size() );
					close( subprocs.fd[W] );

					CGI::wait( subprocs );
					CGI::readFrom( subprocs, out.body );
					CGI::build( out );
				}

				in.reset();
				subprocs.reset();

				srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
			}
		}

		catch ( errstat_t& err ) {
			log( "HTTP\t: transaction: " + str_t( err.what() ) );

			out.msg.str( "" );
			out.body.str( "" );

			HTTP::build( Response( client, err.code ), out );

			in.reset();
			subprocs.reset();
			srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
		}

		catch ( err_t& err ) {
			log( "HTTP\t: Request: " + str_t( err.what() ) );

			HTTP::build( Response( client, 400 ), out );

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

bool
Client::recvMsg( const char* buf, ssize_t& byte_read ) {
	if ( !in.msg_done ) {
		in.msg.write( buf, byte_read );

		size_t pos_header_end = in.msg.str().find( MSG_END, in.msg_read - \
			( in.msg_read * ( in.msg_read < 3 ) + 3 * !( in.msg_read < 3 ) ) );

		if ( !found( pos_header_end ) ) in.msg_read += byte_read;
		else {
			in.msg_done			= TRUE;

			size_t body_begin	= pos_header_end - in.msg_read + 4;
			in.body_read		= byte_read - body_begin;

			if ( in.body_read ) {
				in.body.write( &buf[body_begin], in.body_read );

				// in.msg.seekg( pos_header_end + 4 );
				// in.body.write( in.msg.str().c_str(), in.body_read );
				// char body[in.body_read];
				// in.msg.getline( body, in.body_read );
				// in.msg.seekg( 0 );
				
				// in.msg.str( in.msg.str().substr( 0, pos_header_end + 3 ) );
			}

			// std::clog << "--------------- completed msg ---------------\n" << in.msg.str() << "\n\n\n";

			byte_read			= 0;
		}
	}
	return in.msg_done;
}

bool Client::recvBody(const char* buf, const size_t& byte_read) {
	// keep FALSE untill meet the content-length or tail of chunk (0CRLFCRLF)
	if ( !in.chunk ) return recvBodyPlain( buf, byte_read );
	else return recvBodyChunk( buf, byte_read );
}


bool Client::recvBodyPlain( const char* buf, const size_t& byte_read ) {
	in.body.write( buf, byte_read );
	in.body_read += byte_read;

	if ( in.body_read ) {
		osstream_t oss;
		oss << "TCP\t: body read by " << byte_read << " so far: " << in.body_read << " / " << in.body_size << std::endl;
		log( oss.str() );
	}
	
	return in.body_size == in.body_read;
 }

bool Client::recvBodyChunk( const char* buf, const size_t& byte_read ) {
	// In case of following read for chunk head
	if ( in.incomplete ) {
		std::clog << "recvBodyChunk - incomplete\n";
		// Handle following read is fail, 
		// if ( byte_read != in.next_read ) {
		// 	in.next_read -= byte_read;
		// }

		if ( !subprocs.pid ) in.body.write( buf, byte_read - SIZE_CRLF );
		else CGI::writeTo( subprocs, buf, byte_read - SIZE_CRLF );

		in.incomplete	= FALSE;
		in.next_read	= SIZE_BUFF;
	}

	// In case of first body receiving right after receving message done
	if ( !byte_read && in.body_read ) {
		std::clog << "recvBodyChunk - right after receving message done\n";
		char buf_temp[in.body_read];

		in.body.read( buf_temp, in.body_read );
		in.body.str( "" );

		buf = buf_temp;

		size_t		hex;
		char		data;
		sstream_t	chunk( buf );

		while ( chunk.str().size() ) {
			//  Get chunk head
			chunk >> std::hex >> in.chunk_size >> std::ws;

			// If chunk size is0, the message is end
			if ( !in.chunk_size ) return TRUE;
			if ( in.chunk_size > 0xf ) throw errstat_t( 400, err_msg[CHUNK_EXCEED_HEX] );

			// Write body from chunk 
			hex = in.chunk_size;
			while ( hex && chunk.get( data ) ) {
				if ( !subprocs.pid ) in.body.write( &data, 1 );
				else CGI::writeTo( subprocs, &data, 1 );

				hex--;
			}

			// If unread byte is left, set the next_read
			// as rest of byte and read it following read 
			if ( hex ) {
				in.incomplete	= TRUE;
				in.next_read	= hex + SIZE_CRLF;
				return FALSE;
			}

			if ( chunk.peek() != CR ) throw errstat_t( 400, err_msg[CHUNK_EXCEED_HEX] );

			// If writing body is done well, discard the CRLF and
			// set the next_read with size of next chunk line head
			chunk >> std::ws;
		}
	}

	// In case of getting the chunk head
	else {
		std::clog << "recvBodyChunk - set chunk head\n";

		sstream_t	chunk( buf );

		chunk >> std::hex >> in.chunk_size >> std::ws;
		
		if ( !in.chunk_size ) return TRUE;

		in.next_read	= in.chunk_size;
		in.incomplete	= TRUE;
	}

	return FALSE;
}

bool Client::sendData() 
{
	size_t length = out.msg.str().size(); 

	log( "TCP\t: sending\n" );
	logging.fs << out.msg.str() << "\n" << std::endl;

	ssize_t bytesSent = send(client_socket, out.msg.str().c_str(), length, 0);  
	if (bytesSent < 0) return false;

	length = out.body.str().size(); 


	///////////////////////////////////////////////


	logging.fs << out.body.str() << "\n" << std::endl;

	bytesSent = send(client_socket, out.body.str().c_str(), length, 0);  
	if (bytesSent < 0) return false;

	out.reset();

	if ( rqst ) { delete rqst; rqst = NULL; }
	if ( rspn ) { delete rspn; rspn = NULL; }

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