#ifndef SERVER_HPP
# define SERVER_HPP

# include "Socket.hpp"

# define MAX_CLIENT 32
# define DEFAULT	0

class Server: public Socket {

	public:
		Server(const port_t&);
		Server(Server&&) noexcept;
		~Server();

		Server&	operator=(Server&&) noexcept;

		const config_t&	config(const size_t&) const;
		const vec<config_t>&	config() const;

		void					configAdd(const config_t&);

	private:
		sockaddr_in_t	_addr;
		socklen_t		_addr_len;
		vec<config_t>	_conf;

		void	_open(const port_t&);
		void	_openSetAddr(const int&);
		// void	_openSetNonblock();

};


#endif