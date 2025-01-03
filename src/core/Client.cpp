#include "Client.hpp"
#include "Webserv.hpp"
#include "Transaction.hpp"

/* INSTANCIATE */
Client::Client(const Server& srv):
Socket(srv.sock()), trans(nullptr), _srv(srv) {

	reset();
	setNonblock();
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
	if (this != &source) {
		// do move things
	}
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
	log::print("Client " + std::to_string(sock()) + " receiving");

    ssize_t byte = recv(sock(), _buff, SIZE_BUFF_RECV, 0);
	log::print("Client " + std::to_string(sock()) + " receiving done by " + std::to_string(byte));

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
			not close the connection by just disable more receiving 
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
	}
	catch (err_t& err) {
		log::print("Request: " + str_t(err.what()));

		const errstat_t* errstat = dynamic_cast<const errstat_t*>(&err);
		if (errstat) {
			Transaction::buildError(errstat->code, *this);
		} else {
			Transaction::buildError(400, *this);
		}

		evnt.set(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	}
}

bool Client::_receiveRequestMessage(Kqueue& evnt, ssize_t& byte_recv) {
	if (Transaction::takeHead(in, _buff, byte_recv)) {
		log::history.fs << in.head.str() << std::endl;

		if (!trans) {
			trans = new Transaction(*this, evnt);
		}

		if (Transaction::takeBody(in, subproc, _buff, byte_recv)) {
			log::history.fs << in.body.str() << std::endl;

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
		evnt.set(sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_IDLE, evnt.cast(udata[TIMER_CLIENT_IDLE]));
	} else {
		/*
			When CGI request has received and receving body data has done,
			disable receiving from the client and add process timer
		*/
		close(subproc.fd[W]);

		evnt.set(sock(), EVFILT_READ, EV_DISABLE, 0, 0, evnt.cast(udata[READ_CLIENT]));

		evnt.set(subproc.pid, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_PROC, evnt.cast(sock()));
	}
}

/* METHOD - send: Send the message from out buffer */
bool Client::send() {
	log::print("Client " + std::to_string(sock()) + " sending");

	ssize_t total = 0;
	ssize_t head = _send(out.head);

	if (head < 0) {
		return false;
	}
	total += head;

	if (out.body.peek() != EOF) {
		ssize_t body = _send(out.body);

		if (body < 0) {
			return false;
		}
		total += body;
	}

	log::print("Client " + std::to_string(sock()) + " sending done by " + std::to_string(total));

	return true;
}

ssize_t Client::_send(sstream_t& source) {
	log::history.fs << source.str() << '\n' << std::endl;

	return ::send(sock(), source.str().c_str(), streamsize(source), NONE);
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