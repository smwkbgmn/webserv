#ifndef WEBSERV_HPP
# define WEBSERV_HPP

# include "config.hpp"
# include "Kqueue.hpp"
# include "Server.hpp"
# include "HTTP.hpp"

# include <signal.h>

/* Timeout in ms */
# define TIMEOUT_CLIENT_IDLE 30000
# define TIMEOUT_CLIENT_RQST 30000
# define TIMEOUT_PROCS 60000

enum state_e {
	SUSPEND,
	RUNNING
};

enum udata_e {
	READ_SERVER,
	READ_CLIENT,
	TIMER_CLIENT_IDLE,
	TIMER_CLIENT_RQST
};

extern int udata[4];

class Webserv {
	public:
		Webserv(Kqueue&);
		~Webserv();

		int		state;

		void	init(const char*);
		void	run();
		
	private:
		Kqueue&					_evnt;

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
		void	_initServerCreate(const port_t&);
		void	_initScheme();

		void	_runHandler();
		bool	_runHandlerDisconnect(const event_t&);
		void	_runHandlerRead(const event_t&);
		void	_runHandlerReadServer(const uintptr_t&);
		void	_runHandlerReadClient(const event_t&);
		void	_runHandlerWrite(const event_t&);
		void	_runHandlerProcess(const event_t&);
		void	_runHandlerTimeout(const event_t&);
		void	_runHandlerTimeoutClient(const event_t&, Client&);
		void	_runHandlerTimeoutProcess(const event_t&);

		void	_disconnect(const event_t&);
		void	_disconnectPrint(const event_t&);
};

#endif