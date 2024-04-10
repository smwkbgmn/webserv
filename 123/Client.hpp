#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "ASocket.hpp"
# include "Server.hpp"

class Client: ASocket {
	public:
		const Server&	srv;

		Client( socket_t );
		~Client( void );

		const Server&	server( void ) const { return srv; }
		socket_t	socket( void ) const;

		void		receiving( void );
		void		sending( void );
		
	private:
		Client( void );
};

#endif