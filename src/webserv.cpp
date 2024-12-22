 #include "Webserv.hpp"	

int evnt_udata[2] = {
	EV_UDATA_CONNECTION,
	EV_UDATA_MESSAGE
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

/* Init */
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

			_kq.add(srv.sock(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, reinterpret_cast<void*>(&evnt_udata[EV_UDATA_CONNECTION]));
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

/* Run */
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
	if ((evnt.flags & EV_ERROR) == EV_ERROR) {
		_disconnect(evnt);

		return true;
	}
	if (evnt.flags & EV_EOF
		&& evnt.filter != EVFILT_PROC) {
		// && evnt.filter != EVFILT_WRITE) {

		/*
			One example, in sockets, is when the client disconnects the
			read capacity but leaves the write capacity of the socket in
			place. Than, the EVFILT_WRITE filter (if set) will invoke
			the EV_EOF flag.
		*/

		// if (!_map.sock_cl.at(evnt.ident).cgi) {
			_disconnect(evnt);
		// }

		return true;
	}
	return false;
}

void Webserv::_runHandlerRead(const event_t& evnt) { 
	switch (*reinterpret_cast<int*>(evnt.udata)) {
		case EV_UDATA_CONNECTION	: _runHandlerReadConnect(evnt.ident); break;
		case EV_UDATA_MESSAGE		: _runHandlerReadMessage(evnt); break;
	}
}

void Webserv::_runHandlerReadConnect(const uintptr_t& ident) {
	_list.cl.emplace_back(_map.sock_srv.at(ident));
	Client& cl = _list.cl.back();

	_map.sock_cl.insert(pair<fd_t, Client&>(cl.sock(), cl));

	_kq.add(cl.sock(), EVFILT_READ, EV_ADD | EV_ENABLE, NONE, (intptr_t)0, reinterpret_cast<void*>(&evnt_udata[EV_UDATA_MESSAGE]));
	_kq.add(cl.sock(), EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT, nullptr);
}	

void Webserv::_runHandlerReadMessage(const event_t& evnt) {
	_refreshTimer(evnt.ident);
	_map.sock_cl.at(evnt.ident).receive(_kq);
}

void Webserv::_runHandlerWrite(const event_t& evnt) {
	_refreshTimer(evnt.ident);
	Client& cl = _map.sock_cl.at(evnt.ident);

	if (cl.send()) { 
		if (cl.trans && cl.trans->connection() != CN_CLOSE) {
			cl.reset();
		}
		else if (cl.subproc.pid) {
			kill(cl.subproc.pid, SIGTERM);
			cl.cgi = false;
		}
		else {
			_disconnect(evnt);
		}
	}
	else {
		_disconnect(evnt);
	}
}

///////////////////////////////////////////////////////////

void Webserv::_runHandlerProcess(const event_t& evnt) {
	std::cout << "handler_process\n";

	int udata = *reinterpret_cast<int*>(evnt.udata);
	// std::cout << "pid " + std::to_string(evnt.ident) + " client " + std::to_string(udata) + " \n";
	Client& cl = _map.sock_cl.at(udata);

	try {
		CGI::wait(cl.subproc);
		cl.cgi_exit = true;

		if (WEXITSTATUS(cl.subproc.stat) != EXIT_SUCCESS) {
			throw errstat_t(500, "the CGI failed to exit as SUCCESS");
		}

		CGI::readFrom(cl.subproc, cl.out.body);
		CGI::build(cl.out);

		cl.in.reset();
		cl.subproc.reset();
		cl.cgi_exit = false;

		_kq.add(evnt.ident, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
        _kq.add(evnt.ident, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
		_kq.add(udata ,EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
        _kq.add(udata, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	}
	catch (const errstat_t& e) {
        log::print("TCP\t: " + str_t(e.what()));

		cl.out.reset();
        close(cl.subproc.fd[R]);
		Transaction::buildError(e.code, cl);
		
		cl.cgi = true;

    	_kq.add(cl.subproc.pid, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
        _kq.add(evnt.ident, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
        _kq.add(udata, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	}
}

void Webserv::_runHandlerTimeout(const event_t& evnt) {
	std::cout << "handler_timeout\n";

	/*
		When timed out, the ident could be client or proc_id either
		so need to chekc if it's from Client
	*/

	auto it = _map.sock_cl.find(evnt.ident);

	try {
		if (it != _map.sock_cl.end()) {
			_disconnect(evnt);
		}
		else if (evnt.udata != nullptr) {
			_kq.add(evnt.ident, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
			throw errstat_t(503, "CGI process has timed out");
		}
	}
	catch (const errstat_t& excpt) {
		_runHandlerTimeoutException(evnt, it, excpt);
	}
}

void Webserv::_runHandlerTimeoutException(const event_t& evnt, map<fd_t, Client&>::iterator& it, const errstat_t& excpt) {
	log::print("TCP\t: " + str_t(excpt.what()));

	if (it != _map.sock_cl.end()) {
		it->second.out.reset();
		Transaction::buildError(excpt.code, it->second);
		_kq.add(evnt.ident, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
	}

	/* HAVE NO IDEA */
	else if (evnt.udata != NULL) {
		int udata = *reinterpret_cast<int*>(evnt.udata);
		auto it_udata =_map.sock_cl.find(udata);
		
		if (it_udata != _map.sock_cl.end()) {
			Client& cl = it_udata->second;

			if (cl.subproc.pid) {
				cl.out.reset();
				Transaction::buildError(excpt.code, cl);

				if (cl.cgi_exit) { close(evnt.ident); } // wtf?
				else { kill(evnt.ident, SIGTERM); }

				cl.cgi_exit = false;
				_kq.add(udata, EVFILT_WRITE, EV_ADD | EV_ONESHOT, 0, 0, nullptr);
			}
		}
	}
}

///////////////////////////////////////////////////////////

void Webserv::_refreshTimer(const fd_t& sock) {
	_kq.add(sock, EVFILT_TIMER, EV_ADD | EV_ONESHOT, 0, CL_TIMEOUT, nullptr);
}

void Webserv::_disconnect(const event_t& evnt) {
	if (evnt.filter != EVFILT_TIMER) {
		_kq.add(evnt.ident, EVFILT_TIMER, EV_DELETE, 0, 0, nullptr);
	}
	_kq.add(evnt.ident, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
	_kq.add(evnt.ident, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);

	close(evnt.ident);

	auto it_map = _map.sock_cl.find(evnt.ident);
	auto it_vec = std::find(_list.cl.begin(), _list.cl.end(), it_map->second);
	_list.cl.erase(it_vec);
	_map.sock_cl.erase(it_map);

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
		else {
			cause << "EV_ERROR";
		}
	}

	log::print("Client " + std::to_string(evnt.ident) + " has disconnected due to " + cause.str());
}