#ifndef CGI_HPP
# define CGI_HPP

# include <unistd.h>
# include <new>

# include "HTTP.hpp"

class CGI {
	public:
		static path_t	fileAtidx;

		static void		init( const name_t& );

		static bool		GET( const Request&, char**, size_t& );
		static bool		POST( const Request&, char**, size_t& );

	private:
		static pid_t	_detach( pipe_t[] );
};

#endif