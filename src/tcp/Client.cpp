#include "Client.hpp"
#include "HTTP.hpp"

Client::Client(Server &connect_server) : srv(connect_server), client_socket(-1), Cgi_check(false), Cgi_exit(false) {}

Client::~Client() {}

const Server& Client::getServer() const { return srv; }
const Server& Client::server() const { return srv; }
int* Client::get_client_socket_ptr() { return &client_socket; }
const int& Client::getSocket() const { return client_socket; }
const msg_buffer_t& Client::buffer() const { return in; }
process_t& Client::get_process() { return subprocs; }
// osstream_t& Client::getOss() { return out; }
bool Client::getCgiCheck()  { return Cgi_check; }
bool Client::getCgiExit()  { return Cgi_exit; }
msg_buffer_t& Client::get_in() { return in; }

void Client::setSocket(const int& socket) { client_socket = socket; }
void Client::setServer(const Server& serv) { srv = serv; }
void Client::setCgiCheck(bool value) { Cgi_check = value; }
void Client::setCgiExit(bool value) { Cgi_exit = value; }

void Client::processClientRequest() {

    char buf[SIZE_BUF];

    ssize_t byte = recv(client_socket, buf, SIZE_BUF, 0);
    if (byte < 0) {
        throw err_t("Server socket error on receive");
    } else if (byte == 0) {
        throw err_t("Client receive ended");
    } else {
		try {
			if ( Transaction::recvMsg( in, buf, byte )) {
				logging.fs << in.msg.str() << std::endl;

				if ( !action ) {
					action = new Transaction( *this );

					if ( subprocs.pid ) {
						srv.add_events(subprocs.pid, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, 30000, get_client_socket_ptr());
						srv.add_events(subprocs.pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, get_client_socket_ptr());
					}
				}

				if ( Transaction::recvBody( in, subprocs, buf, byte ) ) {
					logging.fs << in.body.str() << std::endl;

					if ( !subprocs.pid ) action->act();
					else { close( subprocs.fd[W] ); return; }

					in.reset();
					srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
				}
			}
		}

		catch ( errstat_t& err ) {
			log( "HTTP\t: transaction: " + str_t( err.what() ) );

			in.reset();
			out.reset();

			Transaction::build( Response( *this, err.code ), out );
			srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
		}

		catch ( err_t& err ) {
			log( "HTTP\t: Request: " + str_t( err.what() ) );

			in.reset();

			Transaction::build( Response( *this, 400 ), out );
			srv.add_events(client_socket, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, NULL);
		}
    }
}

bool Client::sendData() {
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

msg_buffer_s::msg_buffer_s() { reset(); }

void msg_buffer_s::reset() {
    msg.str("");
    msg.clear();

    msg_done = false;
    msg_read = 0;

    body.str("");
    body.clear();

    body_size = 0;
    body_read = 0;
}

process_s::process_s() { reset(); }

void process_s::reset() {
    pid = NONE;
    stat = NONE;
    fd[R] = NONE;
    fd[W] = NONE;

    argv.clear();
    env.clear();
}
