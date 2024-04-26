#ifndef CGI_HPP
# define CGI_HPP

# include <unistd.h>
# include <sys/wait.h>
# include <new>

# include "HTTP.hpp"

# define SUCCESS 0

typedef stat_t	( *fnptr_t )( const Request&, const process_t& );

const str_t	varPATH_INFO		= "PATH_INFO=";
const str_t varPATH_TRANSLATED	= "PATH_TRANSLATED=";

const str_t	binPHP				= "/usr/bin/php";

class CGI {
	public:
		static void		proceed( const Request&, osstream_t& );

	private:
		static stat_t	_detach( const Request&, process_t&, fnptr_t );

		/* PARENT */
		static void 	_write( const process_t&, const Request& );
		static void		_wait( process_t& );
		static void		_read( process_t&, osstream_t& );

		/* CHILD */
		static bool		_redirect( const process_t& );
		static stat_t	_execve( const process_t&, char**, char** );

		static stat_t	_autoindex( const Request&, const process_t& );
		static stat_t	_script( const Request&, const process_t& );

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

// void
// CGI::_read( process_t& procs, char** bufptr, size_t& size ) {
// 	// osstream_t	oss;
// 	char		buf[1024];

// 	// close( procs.fd[W] );

// 	// std::clog << "_read start read\n";
// 	ssize_t byteRead = 0;

// 	if ( close( procs.fd[W] ) == ERROR ) throwSysErr( "close", 500 );
// 	// while ( LOOP ) {
// 		byteRead = read( procs.fd[R], &buf, 1024 );
// 		buf[byteRead] = '\0';
// 		// close ( procs.fd[R] );

// 		// if ( byteRead == NONE ) break;
// 		// else if ( byteRead == ERROR ) throwSysErr( "read", 500 );
// 		if ( byteRead == ERROR ) throwSysErr( "read", 500 );

// 		*bufptr = strdup( buf );
// 		size = byteRead;

// 		if ( close( procs.fd[R] ) == ERROR ) throwSysErr( "close", 500 );

// 		// std::clog << "[read data from CGI]\n" << buf << ";\n";
// 		// oss << buf;
// 		// std::clog << "[copied data from buf]\n" << oss.str() << ";\n";
// 	// }
	
// 	// while ( read( procs.fd[R], &buf, 1024 ) != NONE ) {
// 	// 	oss << buf;
// 	// 	if ( oss.fail() ) throwSysErr( "read", 500 );
// 	// }
	 
// 	// std::clog << "_read start dupStreamBuffer\n";
// 	// *bufptr = dupStreamBuffer( oss, size );
// }
