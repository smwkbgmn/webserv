#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "config.hpp"
# include "Kqueue.hpp"
# include "Server.hpp"
# include "HTTP.hpp"

# include <signal.h> /* kill */

# define CL_TIMEOUT 50000 // 5s
# define CL_PROC_TIMEOUT 300000 // 30s

enum state_e {
	SUSPEND,
	RUNNING
};

enum evnt_udata_e {
	EV_UDATA_CONNECTION,
	EV_UDATA_MESSAGE
};

// const int evnt_udata[2] = {
// 	EV_UDATA_CONNECTION,
// 	EV_UDATA_MESSAGE
// };

extern int evnt_udata[2];

class Webserv {
	public:
		Webserv(Kqueue&);
		~Webserv();

		int		state;

		void	init(const char*);
		void	run();
		
	private:
		Kqueue&					_kq;

		struct list_s {
			list<Server>		srv;
			list<Client>		cl;
		}						_list;

		struct map_s {
			map<port_t, fd_t>	port_sock;
			map<fd_t, Server&>	sock_srv;
			map<fd_t, Client&>	sock_cl;
		}						_map;

		void	_loadConfig(const char*, vec<config_t>&);
		void	_loadConfigPrint(const vec<config_t>&) const;
		void	_initServer(vec<config_t>&);
		void	_initModule();

		void	_runHandler();
		bool	_runHandlerDisconnect(const event_t&);
		void	_runHandlerRead(const event_t&);
		void	_runHandlerReadConnect(const uintptr_t&);
		void	_runHandlerReadMessage(const event_t&);
		void	_runHandlerWrite(const event_t&);
		void	_runHandlerProcess(const event_t&);
		void	_runHandlerTimeout(const event_t&);
		void	_runHandlerTimeoutException(const event_t&, map<fd_t, Client&>::iterator&, const errstat_t&);

		void	_refreshTimer(const fd_t&);
		void	_disconnect(const event_t&);
		void	_disconnectPrintLog(const event_t&);

};

#endif