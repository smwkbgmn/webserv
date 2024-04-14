#ifndef CGI_HPP
# define CGI_HPP

# include <unistd.h>
# include <sys/wait.h>
# include <new>

# include "HTTP.hpp"

# define SUCCESS 0

typedef struct process_s {
	pid_t	pid;
	pipe_t	fd[2];
	stat_t	stat;

	process_s( void );
}	process_t;

class CGI {
	public:
		typedef stat_t	( *fn_t )( const process_t&, char** );

		static void		init( const name_t& );

		static void		GET( const Request&, char**, size_t& );
		static void		POST( const Request&, char**, size_t& );

	private:
		static void		_argvBuild( vec_cstr_t&, const str_t&, const str_t& );

		
		static stat_t	_detach( process_t&, fn_t, char** );
		static bool		_redirect( const process_t& );
		static stat_t	_execve( const process_t&, char** );

		static stat_t	_autoindex( const process_t&, char** );

		static void		_wait( process_t& );
		static void		_read( process_t&, char**, size_t& );
};


#endif


// void
// CGI::_read( process_t& procs, char** bufptr, size_t& size ) {
// 	char		buf[1024];
// 	str_t		data;
// 	osstream_t	oss;
// 	ssize_t		bytesRead = 0;

// 	bytesRead = read( procs.fd[R], buf, 1024 );
// 	if ( bytesRead == ERROR )
// 		throwSysErr( "read", 500 );
// 	buf[bytesRead] = '\0';

// 	size = bytesRead;
// 	std::clog << "the CGI data[" << size << "]\n" << buf << std::endl;

// 	data.assign( buf );
// 	oss << data;
	
// 	close( procs.fd[R] );

// 	*bufptr = dupStreamBuffer( oss, size );
// 	std::clog << "the copied data\n" << *bufptr << std::endl;
// }
