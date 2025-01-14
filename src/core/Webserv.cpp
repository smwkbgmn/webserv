#include "Webserv.hpp"	

int udata[5] = {
	READ_SERVER,
	READ_CLIENT,
	WRITE_CLIENT_CLOSE,
	TIMER_CLIENT_IDLE,
	TIMER_CLIENT_RQST
};

/* INSTANCIATE */
Webserv::Webserv(Kqueue& event_interface):
state(SUSPEND), _kq(event_interface) {}

Webserv::~Webserv() {}

/* METHOD - init: To be ready for running server */
void Webserv::init(const char* filename) {
	vec<config_t> confs;

	_loadConfig(filename, confs);
	_initServer(confs);
	_initScheme();
}

void Webserv::_loadConfig(const char* filename, vec<config_t>& holder) {
	if (filename) {
		parseConfig(holder, filename);
	} else {
		holder.push_back(config_t());
		holder.front().locations.push_back(location_t(holder.front()));
	}
	_loadConfigPrint(holder);
}

void Webserv::_loadConfigPrint(const vec<config_t>& holder) const {
	std::cout  << "-----------------------------------\n";
	for (auto conf: holder) {
		for (auto it = conf.names.begin(); it != conf.names.end(); ++it) {
			std::cout << *it;
			if (it + 1 != conf.names.end()) {
				std::cout << ' ';
			}
		}
		std::cout << ":" << conf.listen << '\n';

		for (auto loc: conf.locations) {
			std::cout << '\t' << loc.root << ' ';
		}
		std::cout << '\n';
	}
	std::cout  << "-----------------------------------\n";	
}

void Webserv::_initServer(vec<config_t>& confs) {
	for (auto it = confs.begin(); it != confs.end(); ++it) {
		if (_map.port_sock.find(it->listen) == _map.port_sock.end()) {
			_initServerCreate(it->listen);
		} 
		_map.sock_srv.at(_map.port_sock.at(it->listen)).configAdd(*it);
	}
}

void Webserv::_initServerCreate(const port_t& port) {
	_list.srv.emplace_back(port);

	Server& srv = _list.srv.back();

	_map.port_sock.insert(pair<port_t, fd_t>(port, srv.sock()));
	_map.sock_srv.insert(pair<fd_t, Server&>(srv.sock(), srv));

	_kq.set(srv.sock(), EVFILT_READ, EV_ADD, 0, 0, _kq.cast(udata[READ_SERVER]));
}

void Webserv::_initScheme() {
	HTTP::init();
}

/* METHOD - run: Start the server */
void Webserv::run() {
	state = RUNNING;

	while (state == RUNNING) {
		_runHandler();
	}
}

void Webserv::_runHandler() {
	int evnt_new = _kq.renew();

	std::cout << '\n';
	log::print(std::to_string(evnt_new) + " Event");

	for (auto i = 0; i < evnt_new; ++i) {
		if (_runHandlerDisconnect(_kq.event(i))) { continue; }

		switch (_kq.event(i).filter) {
			case EVFILT_READ	: _runHandlerRead(_kq.event(i)); break;
			case EVFILT_WRITE	: _runHandlerWrite(_kq.event(i)); break;
			case EVFILT_PROC	: _runHandlerProcess(_kq.event(i)); break;
			case EVFILT_TIMER	: _runHandlerTimeout(_kq.event(i)); break;
		}
	}
}

bool Webserv::_runHandlerDisconnect(const event_t& ev) {
	if ((ev.flags & EV_EOF && ev.filter != EVFILT_PROC)
		|| ev.flags & EV_ERROR) {
		/*
			One example, in sockets, is when the client disconnects the
			read capacity but leaves the write capacity of the socket in
			place. Than, the EVFILT_WRITE filter (if set) will invoke
			the EV_EOF flag.
		*/
		_disconnect(ev);

		return true;
	}
	return false;
}

void Webserv::_runHandlerRead(const event_t& ev) { 
	switch (_kq.cast(ev.udata)) {
		case READ_SERVER	: _runHandlerReadServer(ev.ident); break;
		case READ_CLIENT	: _runHandlerReadClient(ev); break;
	}
}

void Webserv::_runHandlerReadServer(const uintptr_t& ident) {
	_list.cl.emplace_back(_map.sock_srv.at(ident));

	Client& cl = _list.cl.back();

	_map.sock_cl.insert(pair<fd_t, Client&>(cl.sock(), cl));

	_kq.set(cl.sock(), EVFILT_READ, EV_ADD, 0, 0, _kq.cast(udata[READ_CLIENT]));
	_kq.set(cl.sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_IDLE, _kq.cast(udata[TIMER_CLIENT_IDLE]));
}	

void Webserv::_runHandlerReadClient(const event_t& ev) {
	_kq.set(ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.cast(udata[TIMER_CLIENT_IDLE]));
	_kq.set(ev.ident, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_RQST, _kq.cast(udata[TIMER_CLIENT_RQST]));

	if (!_map.sock_cl.at(ev.ident).receive(_kq)) {
		_disconnect(ev);
	}
}

void Webserv::_runHandlerWrite(const event_t& ev) {
	Client& cl = _map.sock_cl.at(ev.ident);
	/*
		If the connection field has set as close, we need to disconnect Client
		without concern of succecss of sending. For this implementation we need to
		check first if the Reqeust has constructed well, and second what connection
		has set at the header field from both Requeset and Response
	*/
	if (cl.send() && (!cl.trans || cl.trans->connection() != CN_CLOSE) && !ev.udata) { 
		if (cl.out.empty()) {
			cl.reset();
		} else {
			/* The message has not sent all yet. */
			_kq.set(cl.sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		}
		_kq.set(cl.sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_IDLE, _kq.cast(udata[TIMER_CLIENT_IDLE]));
	} else {
		_disconnect(ev);
	}
}

void Webserv::_runHandlerProcess(const event_t& ev) {
	log::print("Client " + std::to_string(_kq.cast(ev.udata)) + " finished CGI");
	/*
		During CGI procedure if the Client has disconnected,
		we will catch and handle it in handler_write
	*/
	Client& cl = _map.sock_cl.at(_kq.cast(ev.udata));

	CGI::wait(cl.subproc);
	if (WEXITSTATUS(cl.subproc.stat) == EXIT_SUCCESS) {
		CGI::read(cl.subproc, cl.out.body);
		CGI::build(cl.out);
	} else {
		Transaction::buildError(500, cl);	
	}

	close(cl.subproc.fd[R]);

	_kq.set(ev.ident, EVFILT_PROC, EV_DELETE, 0, 0, _kq.cast(cl.sock()));
	_kq.set(ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.cast(cl.sock()));

	_kq.set(cl.sock(), EVFILT_READ, EV_ENABLE, 0, 0, _kq.cast(udata[READ_CLIENT]));
	_kq.set(cl.sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	_kq.set(cl.sock() ,EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_IDLE, _kq.cast(udata[TIMER_CLIENT_IDLE]));
}

void Webserv::_runHandlerTimeout(const event_t& ev) {
	/*
		When timed out, the ident may be client_sock and proc_id either.
		So need to chekc if it's from Client or Process by searching map sock_cl.
	*/
	auto it = _map.sock_cl.find(ev.ident);

	if (it != _map.sock_cl.end()) {
		_runHandlerTimeoutClient(ev, it->second);
	} else {
		_runHandlerTimeoutProcess(ev);
	}
}

void Webserv::_runHandlerTimeoutClient(const event_t& ev, Client& cl) {
	log::print("Client " + std::to_string(cl.sock()) + " Proceeding handler timeout");

	if (_kq.cast(ev.udata) == udata[TIMER_CLIENT_IDLE]) {
		_disconnect(ev);
		
	} else {
		Transaction::buildError(408, cl);

		_kq.set(ev.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	}
}

void Webserv::_runHandlerTimeoutProcess(const event_t& ev) {
	log::print("Client " + std::to_string(_kq.cast(ev.udata)) + " Proceeding handler timeout");

	Client& cl = _map.sock_cl.at(_kq.cast(ev.udata));
	/*
		Because of no need to proceed futher process, kill
		the process and collect exit status by calling wait.
		It also prevent the process from being defunct(a.k.a zombie).
	*/
	kill(ev.ident, SIGTERM);
	CGI::wait(cl.subproc);

	close(cl.subproc.fd[R]);

	Transaction::buildError(504, cl);

	_kq.set(ev.ident, EVFILT_PROC, EV_DELETE, 0, 0, nullptr);

	_kq.set(cl.sock(), EVFILT_READ, EV_ENABLE, 0, 0, _kq.cast(udata[READ_CLIENT]));
	_kq.set(cl.sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	_kq.set(cl.sock() ,EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, TIMEOUT_CLIENT_IDLE, _kq.cast(udata[TIMER_CLIENT_IDLE]));	
}

void Webserv::_disconnect(const event_t& ev) {
	auto it_map = _map.sock_cl.find(ev.ident);
	auto it_lst = std::find(_list.cl.begin(), _list.cl.end(), it_map->second);

	_map.sock_cl.erase(it_map);
	_list.cl.erase(it_lst);

	if (ev.filter != EVFILT_TIMER) {
		_kq.set(ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.cast(udata[TIMER_CLIENT_IDLE]));
		_kq.set(ev.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.cast(udata[TIMER_CLIENT_RQST]));
	}
	_kq.set(ev.ident, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
	_kq.set(ev.ident, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

	_disconnectPrint(ev);
}

void Webserv::_disconnectPrint(const event_t& ev) {
	sstream_t cause;

	switch (ev.filter) {
		case EVFILT_TIMER	: cause << "EVFILT_TIMER"; break;
		case EVFILT_WRITE	: {
			if (ev.udata && _kq.cast(ev.udata) == udata[WRITE_CLIENT_CLOSE]) {
				/*
					Above err_status code 410 means that it may include 
					following body data receiving. To avoid unnecessary 
					receiving, flaged it as closing connection.
				*/
				cause << "ERR_STATUS_ABOVE_410"; break;
			}
		}

		default: {
			if (ev.flags & EV_EOF) {
				cause << "EV_EOF";
			} else if (ev.flags & EV_ERROR) {
				cause << "EV_ERROR";
			} else {
				cause << "UNKNOWN";
			}
		}
	}

	log::print("Client " + std::to_string(ev.ident) + " disconnected due to " + cause.str());
}