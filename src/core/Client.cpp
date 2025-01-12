#include "Client.hpp"
#include "Webserv.hpp"
#include "Transaction.hpp"

/* INSTANCIATE */
Client::Client(const Server& srv):
Socket(srv.sock()), trans(nullptr), _srv(srv) {

	reset();
	setNonblock();

	log::print("Client " + std::to_string(sock()) + " connected to Server " + std::to_string(srv.sock()));
}

Client::Client(Client&& source) noexcept:
Socket(std::move(source)), trans(source.trans), _srv(source._srv) {
		
	in = std::move(source.in);
	out = std::move(source.out);
	subproc = std::move(source.subproc);

	source.trans = nullptr;
}

Client::~Client() {
	if (trans) { delete trans; }
}

/* OPERATOR */
Client& Client::operator=(Client&& source) noexcept {
	if (this != &source) {}

	return *this;
}

bool Client::operator==(const Client& source) const {
	return sock() == source.sock();
}

/* ACCESS */
const Server& Client::server() const {
	return _srv;
}

/* METHOD - receive: Receive message and handle it with in buffer */
bool Client::receive(Kqueue& evnt) {
    ssize_t byte = recv(sock(), _buff, SIZE_BUFF_RECV, 0);

	log::print("Client " + std::to_string(sock()) + " received by " + std::to_string(byte));

	if (byte > 0) {
		_receiveRequest(evnt, byte);

		return true;
	} else if (byte == 0) {
		/*
			The client has gracefully closed the connection.
			This means the client has sent a FIN packet to signal that
			it will not send any more data.

			Because the client may still be able to receive data from
			the server (if it has not closed its receiving side), should
			not close the connection by just disable any futher receiving 
			(e.g. The rest part of data that is produced through CGI)
		*/
		evnt.set(sock(), EVFILT_READ, EV_DISABLE, 0, 0, evnt.cast(udata[READ_CLIENT]));

		return true;
	} else if (errno == EAGAIN || errno == EWOULDBLOCK) {
		/*
			If errno is EAGAIN or EWOULDBLOCK, the socket is set to
			non-blocking mode, and no data is available to read at the moment
			and it means that it's not error while should wait for new read event
		*/
		return true;
	} else {
		return false;
	}
}

void Client::_receiveRequest(Kqueue& evnt, ssize_t& byte_recv) {
	try {
		if (_receiveRequestMessage(evnt, byte_recv)) {
			_receiveRequestDo(evnt);
		}
	} catch (err_t& err) {
		_receiveRequestFail(evnt, err);
	}
}

bool Client::_receiveRequestMessage(Kqueue& evnt, ssize_t& byte_recv) {
	if (Transaction::takeHead(in, _buff, byte_recv)) {
		if (!trans) {
			trans = new Transaction(*this, evnt);
		}
		if (Transaction::takeBody(in, subproc, _buff, byte_recv)) {
			return true;
		}
	}
	return false;
}

void Client::_receiveRequestDo(Kqueue& evnt) {
	evnt.set(sock(), EVFILT_TIMER, EV_DELETE, 0, 0, evnt.cast(udata[TIMER_CLIENT_RQST]));

	if (!subproc.pid) {
		if (trans) {
			trans->act();
		}

		evnt.set(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		evnt.set(sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_IDLE, evnt.cast(udata[TIMER_CLIENT_IDLE]));
	} else {
		/*
			When CGI request has received and receving body data has done,
			disable receiving from the client and add process timer
		*/
		close(subproc.fd[W]);

		evnt.set(sock(), EVFILT_READ, EV_DISABLE, 0, 0, evnt.cast(udata[READ_CLIENT]));

		evnt.set(subproc.pid, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_PROCS, evnt.cast(sock()));
	}
}

void Client::_receiveRequestFail(Kqueue& evnt, err_t& err) {
	std::cerr << "Request: " + str_t(err.what()) << '\n';

	const errstat_t* errstat = dynamic_cast<const errstat_t*>(&err);
	if (errstat) {
		Transaction::buildError(errstat->code, *this);
	} else {
		Transaction::buildError(400, *this);
	}

	evnt.set(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
}

/* METHOD - send: Send the message from out buffer */
bool Client::send() {
	ssize_t total = 0;
	
	if (out.head.peek() != EOF
		&& !_sendMessage(out.head, total)) {
		return false;
	}

	if (out.head.peek() == EOF && out.body.peek() != EOF
		&& !_sendMessage(out.body, total)) {
		return false;
	}

	log::print("Client " + std::to_string(sock()) + " sent by " + std::to_string(total));
	return true;
}

bool Client::_sendMessage(sstream_t& source, ssize_t& counter) {
	size_t pos = static_cast<size_t>(source.tellg());

	ssize_t sent = _send(source);
	if (sent < 0) {
		return false;
	}
	counter += sent;

	source.seekg(static_cast<streamoff_t>(pos + sent), std::ios_base::beg);
	return true;
}

ssize_t Client::_send(sstream_t& source) {
	size_t size = streamsize(source);

	char* buff = new char[size];
	source.read(buff, size);
	
	ssize_t sent = ::send(sock(), buff, size, NONE);
	delete[] buff;

	return sent;
}

void Client::reset() {
	in.reset();
	out.reset();

	if (trans) {
		delete trans;
		trans = NULL;
	}
	subproc.reset();
}