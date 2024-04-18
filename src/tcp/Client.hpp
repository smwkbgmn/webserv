#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ASocket.hpp"
# include "Server.hpp"

class Client: ASocket {
	public:
		char	buf[1024];
		
		Client( socket_t, const Server& );
		~Client( void );

		socket_t		socket( void ) const { return sock; }
		const Server&	server( void ) const { return _server; }
		const char*		buffer( void ) const { return buf; }

		void		receiving( void );
		void		sending( void );
		
	private:
	 	const Server&	_server;
		Client( void );
};

#endif