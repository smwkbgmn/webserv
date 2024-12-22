#include "Client.hpp"
#include "Webserv.hpp"
#include "Transaction.hpp"

/* INSTANCIATE */
Client::Client(const Server& srv):
	Socket(srv.sock()),
	trans(nullptr),
	cgi(false),
	cgi_exit(false),
	_srv(srv) {

	setNonblock();
}

Client::Client(Client&& source) noexcept:
	Socket(std::move(source)),
	trans(source.trans),
	in(std::move(source.in)),
	out(std::move(source.out)),
	subproc(std::move(source.subproc)),
	cgi(source.cgi),
	cgi_exit(source.cgi_exit),
	_srv(source._srv) {

	source.trans = nullptr;
}

Client::~Client() {
	if (trans) {
		delete trans;
	}
}
/* OPERATOR OVERLOAD */
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

/* METHOD */
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
			// cgi = true;

			// kq.add(sock(), EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, reinterpret_cast<void*>(&evnt_udata[EV_UDATA_MESSAGE]));
			kq.add(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		}
	}
	else if (byte == 0) {
		kq.add(sock(), EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, reinterpret_cast<void*>(&evnt_udata[EV_UDATA_MESSAGE]));
	}
	else {
		throw err_t("Server socket error on receive");
	}
	return byte;
}

void Client::_receiveTransaction(Kqueue& kq, ssize_t& byte_recv) {
	if (Transaction::recvHead(in, _buff, byte_recv)) {
		log::history.fs << in.head.str() << std::endl;

		if (!trans) {
			trans = new Transaction(*this);

			if (subproc.pid) {
				/*
					When CGI request has received, disable requests from the client 
					and remove the client timer by replacing it with process timer
				*/
				kq.add(sock(), EVFILT_TIMER, EV_DELETE | EV_ONESHOT, 0, 0, nullptr);
				kq.add(sock(), EVFILT_READ, EV_ADD | EV_DISABLE, 0, 0, reinterpret_cast<void*>(&evnt_udata[EV_UDATA_MESSAGE]));
				kq.add(subproc.pid, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_PROC_TIMEOUT, reinterpret_cast<void*>(const_cast<int*>(&sock())));
				kq.add(subproc.pid, EVFILT_PROC, EV_ADD | EV_ONESHOT, NOTE_EXIT, 0, reinterpret_cast<void*>(const_cast<int*>(&sock())));
			}
		}

		if (Transaction::recvBody(in, subproc, _buff, byte_recv)) {
			log::history.fs << in.body.str() << std::endl;

			if (!subproc.pid) {
				if (trans) { trans->act(); }

				in.reset();
				kq.add(sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
			}
			else { close(subproc.fd[W]); }
		}
	}
}

bool Client::send() {
	log::print("sending");

	ssize_t byte = _send(out.head);
	if (byte < 0) {
		return false;
	}

	if (out.body.peek() != EOF) {
		ssize_t byte_body = _send(out.body);

		if (byte_body < 0) {
			return false;
		}
		byte += byte_body;
	}

	log::print("sending done by " + std::to_string(byte));

	return true;
}

ssize_t Client::_send(sstream_t& source) {
	log::history.fs << source.str() << '\n' << std::endl;

	return ::send(sock(), source.str().c_str(), streamsize(source), NONE);
}

void Client::reset() {
	_resetTransaction();
	in.reset();
	out.reset();
}

void Client::_resetTransaction() {
	if (trans) {
		delete trans;
		trans = NULL;
	}
}

