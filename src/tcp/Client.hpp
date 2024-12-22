#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "CGI.hpp"
# include "Server.hpp"
# include "Transaction.hpp"

class Kqueue;
// struct proces_t;

class Client: public Socket {
	
	public:
		Client() = delete;
		Client(const Server&);
		Client(Client&&) noexcept;
		~Client();

		// For move sementics to container
		Client&			operator=(Client&&) noexcept;
		// For searching map container sock_Client
		bool			operator==(const Client&) const;

		const Server&	server() const;

		Transaction*	trans;
		message_t		in;
		message_t		out;
		
		process_t		subproc;

		// what for? //////////
		bool			cgi;
		bool			cgi_exit;
		///////////////////////

		ssize_t			receive(Kqueue&);
		bool			send();
		void			reset();

	private:
		const Server&	_srv;
		char			_buff[SIZE_BUFF_RECV];
	
		void			_receiveTransaction(Kqueue&, ssize_t&);

		ssize_t			_send(sstream_t&);

		void			_resetTransaction();
};

#endif