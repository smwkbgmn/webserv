#ifndef SOCKET_HPP
# define SOCKET_HPP

# include "type.hpp"

# include <unistd.h>
# include <fcntl.h>

# include <netinet/in.h>
# include <sys/socket.h>

typedef int fd_t;

typedef struct sockaddr		sockaddr_t;
typedef struct sockaddr_in	sockaddr_in_t;

class Socket {

	public:
		Socket(); // Open Server
		Socket(const fd_t&); // Accept Client
		Socket(const Socket&&) noexcept;
		virtual ~Socket();

		Socket&			operator=(const Socket&&) noexcept;

		sockaddr_in_t	addr;
		socklen_t		addr_len;

		const fd_t&		sock() const;
		void			setNonblock() const;

	private:	
		fd_t 			_sock;

};

#endif