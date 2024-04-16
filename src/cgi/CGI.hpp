#ifndef CGI_HPP
# define CGI_HPP

# include <unistd.h>
# include <sys/wait.h>
# include <new>

# include "HTTP.hpp"

# define SUCCESS 0

typedef stat_t	( *fnptr_t )( const Request&, const process_t& );

const str_t	varPATH_INFO	= "PATH_INFO=";

const str_t	binPHP			= "/usr/bin/php";

class CGI {
	public:
		static void		proceed( const Request&, char**, size_t& );
		// static void		GET( const Request&, char**, size_t& );
		// static void		POST( const Request&, char**, size_t& );

	private:
		static stat_t	_detach( const Request&, process_t&, fnptr_t );

		/* PARENT */
		static void		_wait( process_t& );
		static void		_read( process_t&, char**, size_t& );

		/* CHILD */
		static bool		_redirect( const process_t& );
		static stat_t	_execve( const process_t&, char**, char** );

		static stat_t	_autoindex( const Request&, const process_t& );

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
