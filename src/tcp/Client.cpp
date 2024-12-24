#include "Client.hpp"
#include "Webserv.hpp"
#include "Transaction.hpp"

/* INSTANCIATE */
Client::Client(const Server& srv):
	Socket(srv.sock()),
	trans(nullptr),
	_srv(srv) {

	reset();
	setNonblock();
}

Client::Client(Client&& source) noexcept:
	Socket(std::move(source)),
	trans(source.trans),
	_srv(source._srv) {

	in = std::move(source.in);
	out = std::move(source.out);
	subproc = std::move(source.subproc);

	source.trans = nullptr;
}

Client::~Client() {
	if (trans) {
		delete trans;
	}
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
ssize_t Client::receive(Kqueue& kq) {
	log::print("receiving");

    ssize_t byte = recv(sock(), _buff, SIZE_BUFF_RECV, 0);
	log::print("receiving done by " + std::to_string(byte));

	if (byte > 0) {
		try { _receiveTransaction(kq, byte); }
		catch (err_t& err) {
			log::print("Request: " + str_t(err.what()));

			const errstat_t* errstat = dynamic_cast<const errstat_t*>(&err);
			if (errstat) {
				Transaction::buildError(errstat->code, *this);
			}
			else {
				Transaction::buildError(400, *this);
			}

			kq.set(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		}
	}
	return byte;
}

void Client::_receiveTransaction(Kqueue& kq, ssize_t& byte_recv) {
	if (Transaction::recvHead(in, _buff, byte_recv)) {
		log::history.fs << in.head.str() << std::endl;

		if (!trans) {
			trans = new Transaction(*this);
		}

		if (Transaction::recvBody(in, subproc, _buff, byte_recv)) {
			log::history.fs << in.body.str() << std::endl;

			_doRequestedAction(kq);
		}
	}
}

void Client::_doRequestedAction(Kqueue& kq) {
	kq.set(sock(), EVFILT_TIMER, EV_DELETE, 0, 0, kq.castUdata(udata[UDT_TIMER_CLIENT_RQST]));

	if (!subproc.pid) {
		if (trans) {
			trans->act();
		}

		kq.set(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		kq.set(sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_IDLE, kq.castUdata(udata[UDT_TIMER_CLIENT_IDLE]));
	}
	else {
		/*
			When CGI request has received and receving body data has done,
			disable receiving from the client and remove the client timer
			by replacing it with process timer
		*/
		kq.set(sock(), EVFILT_READ, EV_DISABLE, 0, 0, kq.castUdata(udata[UDT_READ_CLIENT]));

		kq.set(subproc.pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, kq.castUdata(sock()));
		kq.set(subproc.pid, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_PROC, kq.castUdata(sock()));

		close(subproc.fd[W]);
	}
}

/* METHOD - send: Send the message from out buffer */
bool Client::send() {
	log::print("sending");

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

	log::print("sending done by " + std::to_string(total));

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