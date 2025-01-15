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
bool Client::receive(Kqueue& kq) {
    ssize_t byte = recv(sock(), _buff, SIZE_BUFF_RECV, 0);

	log::print("Client " + std::to_string(sock()) + " received by " + std::to_string(byte));

	if (byte > 0) {
		_receiveRequest(kq, byte);

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
		kq.set(sock(), EVFILT_READ, EV_DISABLE, 0, 0, kq.cast(udata[READ_CLIENT]));

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

void Client::_receiveRequest(Kqueue& kq, ssize_t& byte_recv) {
	try {
		if (_receiveRequestMessage(kq, byte_recv)) {
			_receiveRequestDo(kq);
		}
	} catch (err_t& err) {
		_receiveRequestFail(kq, err);
	}
}

bool Client::_receiveRequestMessage(Kqueue& kq, ssize_t& byte_recv) {
	if (!in.head_done && Transaction::takeHead(in, _buff, byte_recv)) {
		trans = new Transaction(*this);

		trans->checkTarget();
		trans->checkCGI(kq);
	}
	if (trans && Transaction::takeBody(in, subproc, _buff, byte_recv)) {
		return true;
	}
	return false;
}

void Client::_receiveRequestDo(Kqueue& kq) {
	kq.set(sock(), EVFILT_TIMER, EV_DELETE, 0, 0, kq.cast(udata[TIMER_CLIENT_RQST]));

	if (!subproc.pid) {
		trans->act();

		kq.set(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		kq.set(sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_IDLE, kq.cast(udata[TIMER_CLIENT_IDLE]));
	} else {
		/*
			When CGI request has received and receving body data has done,
			disable receiving from the client and add process timer
		*/
		close(subproc.fd[W]);

		kq.set(sock(), EVFILT_READ, EV_DISABLE, 0, 0, kq.cast(udata[READ_CLIENT]));

		kq.set(subproc.pid, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_PROCS, kq.cast(sock()));
	}
}

void Client::_receiveRequestFail(Kqueue& kq, err_t& err) {
	std::cerr << "Request: " + str_t(err.what()) << '\n';

	const errstat_t* errstat = dynamic_cast<const errstat_t*>(&err);
	void* udata_close = nullptr;
	
	if (errstat) {
		Transaction::buildError(errstat->code, *this);

		if (errstat->code > 410) {
			udata_close = kq.cast(udata[WRITE_CLIENT_CLOSE]);

			kq.set(sock(), EVFILT_READ, EV_DISABLE, 0, 0, kq.cast(udata[READ_CLIENT]));
		}
	} else {
		Transaction::buildError(400, *this);
	}

	kq.set(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, udata_close);
	kq.set(sock(), EVFILT_TIMER, EV_DELETE, 0, 0, kq.cast(udata[TIMER_CLIENT_RQST]));
	kq.set(sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_IDLE, kq.cast(udata[TIMER_CLIENT_IDLE]));
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

bool Client::_sendMessage(sstream_t& source, ssize_t& total) {
	size_t pos = static_cast<size_t>(source.tellg());

	ssize_t sent = _send(source);
	if (sent < 0) {
		return false;
	}
	total += sent;

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