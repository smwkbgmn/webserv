#ifndef SERVER_HPP
# define SERVER_HPP

# include "ASocket.hpp"
# include "Client.hpp"

class Server: ASocket {
	public:
		config_t	cnf;
	
		Server( void );
		~Server( void );

		void	listening( void );

		const config_t&	config( void ) const { return cnf; }

	private:
		Server( socket_t );
};

#endif