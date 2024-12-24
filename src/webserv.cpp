 #include "Webserv.hpp"	

int udata[4] = {
	UDT_READ_SERVER,
	UDT_READ_CLIENT,
	UDT_TIMER_CLIENT_IDLE,
	UDT_TIMER_CLIENT_RQST
};

/* INSTANCIATE */
Webserv::Webserv(Kqueue& event_interface): state(SUSPEND), _kq(event_interface) {
	log::print("Constructing Core Module...");
}

Webserv::~Webserv() {
	for (auto it = _list.cl.begin(); it != _list.cl.end(); ++it) {
		close(it->sock());
	}
}

/* METHOD - init: To be ready to run server */
void Webserv::init(const char* filename) {
	vec<config_t> confs;

	_loadConfig(filename, confs);
	_initServer(confs);
	_initModule();
}

void Webserv::_loadConfig(const char* filename, vec<config_t>& holder) {
	log::print("Parsing Config...");

	if (filename) { configParse(holder, filename); }
	else {
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
	log::print("Initiating Server...");

	for (auto it = confs.begin(); it != confs.end(); ++it) {
		if (_map.port_sock.find(it->listen) == _map.port_sock.end()) {
			_list.srv.emplace_back(it->listen);
			Server& srv = _list.srv.back();

			srv.configAdd(*it);

			_map.port_sock.insert(pair<port_t, fd_t>(it->listen, srv.sock()));
			_map.sock_srv.insert(pair<fd_t, Server&>(srv.sock(), srv));

			_kq.set(srv.sock(), EVFILT_READ, EV_ADD, 0, 0, _kq.castUdata(udata[UDT_READ_SERVER]));
		}
		else {
			_map.sock_srv.at(_map.port_sock.at(it->listen)).configAdd(*it);
		}
	}
}

void Webserv::_initModule() {
	HTTP::init();
	CGI::init();
}

/* METHOD - run: Start the server */
void Webserv::run() {
	log::print("Server Has Started");

	state = RUNNING;
	while (state) { _runHandler(); }
}

void Webserv::_runHandler() {
	int evnt_new = _kq.renew();

	std::cout << '\n';
	log::print(std::to_string(evnt_new) + " Event");

	for (auto i = 0; i < evnt_new; ++i) {
		if (_runHandlerDisconnect(_kq.que(i))) { continue; }

		switch (_kq.que(i).filter) {
			case EVFILT_READ	: _runHandlerRead(_kq.que(i)); break;
			case EVFILT_WRITE	: _runHandlerWrite(_kq.que(i)); break;
			case EVFILT_PROC	: _runHandlerProcess(_kq.que(i)); break;
			case EVFILT_TIMER	: _runHandlerTimeout(_kq.que(i)); break;
		}
	}
}

bool Webserv::_runHandlerDisconnect(const event_t& evnt) {
	if ((evnt.flags & EV_EOF && evnt.filter != EVFILT_PROC)
		|| evnt.flags & EV_ERROR) {
		/*
			One example, in sockets, is when the client disconnects the
			read capacity but leaves the write capacity of the socket in
			place. Than, the EVFILT_WRITE filter (if set) will invoke
			the EV_EOF flag.
		*/
		_disconnect(evnt);

		return true;
	}
	return false;
}

void Webserv::_runHandlerRead(const event_t& evnt) { 
	switch (_kq.castUdata(evnt.udata)) {
		case UDT_READ_SERVER	: _runHandlerReadServer(evnt.ident); break;
		case UDT_READ_CLIENT	: _runHandlerReadClient(evnt); break;
	}
}

void Webserv::_runHandlerReadServer(const uintptr_t& ident) {
	_list.cl.emplace_back(_map.sock_srv.at(ident));
	Client& cl = _list.cl.back();

	_map.sock_cl.insert(pair<fd_t, Client&>(cl.sock(), cl));

	_kq.set(cl.sock(), EVFILT_READ, EV_ADD, 0, 0, _kq.castUdata(udata[UDT_READ_CLIENT]));
	_kq.set(cl.sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_IDLE, _kq.castUdata(udata[UDT_TIMER_CLIENT_IDLE]));
}	

void Webserv::_runHandlerReadClient(const event_t& evnt) {
	_kq.set(evnt.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.castUdata(udata[UDT_TIMER_CLIENT_IDLE]));
	_kq.set(evnt.ident, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_RQST, _kq.castUdata(udata[UDT_TIMER_CLIENT_RQST]));

	ssize_t byte = _map.sock_cl.at(evnt.ident).receive(_kq);
	if (byte == 0) {
		/*
			The client has gracefully closed the connection.
			This means the client has sent a FIN packet to signal that
			it will not send any more data.

			Because the client may still be able to receive data from
			the server (if it has not closed its receiving side), should
			not close the connection by just disable more receiving 
			(e.g. The rest part of data that is produced through CGI)
		*/
		_kq.set(evnt.ident, EVFILT_READ, EV_DISABLE, 0, 0, _kq.castUdata(udata[UDT_READ_CLIENT]));
	}
	/*
		If errno is EAGAIN or EWOULDBLOCK, the socket is set to
		non-blocking mode, and no data is available to read at the moment
		and it means that it's not error while should wait for new read event
	*/
	else if (byte < 0 && !(errno == EAGAIN || errno == EWOULDBLOCK)) {
		_disconnect(evnt);
	}
}

void Webserv::_runHandlerWrite(const event_t& evnt) {
	Client& cl = _map.sock_cl.at(evnt.ident);

	if (cl.send() || (cl.trans && cl.trans->connection() != CN_CLOSE)) { 
		cl.reset();

		_kq.set(cl.sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_IDLE, _kq.castUdata(udata[UDT_TIMER_CLIENT_IDLE]));
	}
	else {
		_disconnect(evnt);
	}
}

void Webserv::_runHandlerProcess(const event_t& evnt) {
	log::print("proceeding CGI done");
	/*
		If while CGI procedure the Client has disconnected,
		we will catch and handle it in write handler
	*/
	Client& cl = _map.sock_cl.at(_kq.castUdata(evnt.udata));

	try {
		CGI::wait(cl.subproc);

		if (WEXITSTATUS(cl.subproc.stat) != EXIT_SUCCESS) {
			throw errstat_t(500, err_msg[CGI_EXIT_FAILURE]);
		}

		CGI::readFrom(cl.subproc, cl.out.body);
		CGI::build(cl.out);
	}
	catch (const errstat_t& e) {
        log::print(str_t(e.what()));

		Transaction::buildError(e.code, cl);
	}

	close(cl.subproc.fd[R]);

	_kq.set(cl.sock(), EVFILT_READ, EV_ENABLE, 0, 0, _kq.castUdata(udata[UDT_READ_CLIENT]));
	_kq.set(cl.sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	_kq.set(cl.sock() ,EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_IDLE, _kq.castUdata(udata[UDT_TIMER_CLIENT_IDLE]));

	_kq.set(evnt.ident, EVFILT_PROC, EV_DELETE, 0, 0, _kq.castUdata(cl.sock()));
	_kq.set(evnt.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.castUdata(cl.sock()));
}

void Webserv::_runHandlerTimeout(const event_t& evnt) {
	log::print("Proceeding handler timeout");
	/*
		When timed out, the ident could be client_sock or proc_id either
		So need to chekc if it's from Client or Process by searching map sock_cl
	*/
	auto it = _map.sock_cl.find(evnt.ident);

	if (it != _map.sock_cl.end()) {
		if (_kq.castUdata(evnt.udata) == udata[UDT_TIMER_CLIENT_IDLE]) {
			_disconnect(evnt);
		}
		else {
			Transaction::buildError(408, it->second);
			
			_kq.set(evnt.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		}
	}
	else {
		Client& cl = _map.sock_cl.at(_kq.castUdata(evnt.udata));

		Transaction::buildError(504, cl);

		_kq.set(evnt.ident, EVFILT_PROC, EV_DELETE, 0, 0, nullptr);

		_kq.set(cl.sock(), EVFILT_READ, EV_ENABLE, 0, 0, _kq.castUdata(udata[UDT_READ_CLIENT]));
		_kq.set(cl.sock(), EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
		_kq.set(cl.sock() ,EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT_IDLE, _kq.castUdata(udata[UDT_TIMER_CLIENT_IDLE]));
	}
}

void Webserv::_disconnect(const event_t& evnt) {
	close(evnt.ident);

	auto it_map = _map.sock_cl.find(evnt.ident);
	auto it_vec = std::find(_list.cl.begin(), _list.cl.end(), it_map->second);
	_list.cl.erase(it_vec);
	_map.sock_cl.erase(it_map);

	if (evnt.filter != EVFILT_TIMER) {
		_kq.set(evnt.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.castUdata(udata[UDT_TIMER_CLIENT_IDLE]));
		_kq.set(evnt.ident, EVFILT_TIMER, EV_DELETE, 0, 0, _kq.castUdata(udata[UDT_TIMER_CLIENT_RQST]));
	}
	_kq.set(evnt.ident, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
	_kq.set(evnt.ident, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

	_disconnectPrintLog(evnt);
}

void Webserv::_disconnectPrintLog(const event_t& evnt) {
	sstream_t cause;

	if (evnt.filter == EVFILT_TIMER) {
		cause << "EVFILT_TIMER";
	}
	else {
		if (evnt.flags & EV_EOF) {
			cause << "EV_EOF";
		}
		else if (evnt.flags & EV_ERROR) {
			cause << "EV_ERROR";
		}
		else {
			cause << "EV_UNKNOWN";
		}
	}

	log::print("Client " + std::to_string(evnt.ident) + " has disconnected due to " + cause.str());
}