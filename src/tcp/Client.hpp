#ifndef CLIENT_HPP
# define CLIENT_HPP

# include "CGI.hpp"
# include "Server.hpp"
# include "Transaction.hpp"

class Kqueue;

class Client: public Socket {
	public:
		Client() = delete;
		Client(const Server&);
		Client(Client&&) noexcept;
		~Client();

		Client&			operator=(Client&&) noexcept;
		/* For searching map container sock_Client */
		bool			operator==(const Client&) const;

		const Server&	server() const;

		message_t		in;
		message_t		out;

		Transaction*	trans;
		process_t		subproc;

		bool			receive(Kqueue&);
		bool			send();
		void			reset();

	private:
		const Server&	_srv;
		char			_buff[SIZE_BUFF_RECV];
	
		void			_receiveTransaction(Kqueue&, ssize_t&);
		bool			_receiveTransactionMessage(ssize_t&);
		void			_receiveTransactionDo(Kqueue&);

		ssize_t			_send(sstream_t&);
};

#endif