#include "CGI.hpp"

#include <cstring>

// autoindexing (GET/.php), RPN calculator (GET/.exe), sorting (POST/.cpp)

void
CGI::GET( const Request& rqst, char** bufptr, size_t& size ) {
	process_t	procs;
	vec_str_t	argv;
	// argv.push_back( "/usr/bin/php" );
	// argv.push_back( rqst.config().root + HTTP::http.fileAtidx );
	vec_cstr_t	cargv;


	// autoindex.cgi
	if ( *rqst.line().uri.rbegin() == '/' ) {
		// for ( vec_str_t::const_iterator iter = argv.begin(); iter != argv.end(); ++iter )
		// 	cargv.push_back( const_cast<char*>( iter->c_str() ) );
		// cargv.push_back( NULL );

		if ( _detach( rqst, procs, &_autoindex ) == SUCCESS ) {
			_wait( procs );
			_read( procs, bufptr, size );
			
			if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS ) throw errstat_t( 500 );
		}
		else throwSysErr( "execve", 500 );
	}
}

void
CGI::POST( const Request& rqst, char** bufptr, size_t& size ) {
	( void )rqst;
	( void )bufptr;
	( void )size;
}

stat_t
CGI::_detach( const Request& rqst, process_t& procs, fnptr_t execute ) {
	if ( pipe( procs.fd ) == ERROR || ( procs.pid = fork() ) == ERROR )
		throwSysErr( "_detach", 500 );
	
	if ( !procs.pid ) return execute( rqst, procs );
	else return SUCCESS;
}

// Be aware that functions under _detach are running on subprocess
bool
CGI::_redirect( const process_t& procs ) {
	if ( dup2( procs.fd[W], STDOUT_FILENO ) == ERROR  ||
		close( procs.fd[R] ) == ERROR || close( procs.fd[W] ) == ERROR ) {
			perror( "_redirect" );
			return FALSE;
	}
	return TRUE;	
}

stat_t
CGI::_execve( const process_t& procs, char* argv[] ) {
	// std::cerr << "child > _redirect\n";
	if ( _redirect( procs ) ) return execve( *argv, argv, NULL );
	return EXIT_FAILURE;
}

void
CGI::_wait( process_t& procs ) {
	if ( waitpid( procs.pid, &procs.stat, NONE ) == ERROR )
		throwSysErr( "wait", 500 );
}
 
void
CGI::_read( process_t& procs, char** bufptr, size_t& size ) {
	// osstream_t	oss;
	char		buf[1024];

	// close( procs.fd[W] );

	// std::clog << "_read start read\n";
	ssize_t byteRead = 0;

	if ( close( procs.fd[W] ) == ERROR ) throwSysErr( "close", 500 );
	// while ( LOOP ) {
		byteRead = read( procs.fd[R], &buf, 1024 );
		buf[byteRead] = '\0';
		// close ( procs.fd[R] );

		// if ( byteRead == NONE ) break;
		// else if ( byteRead == ERROR ) throwSysErr( "read", 500 );
		if ( byteRead == ERROR ) throwSysErr( "read", 500 );

		*bufptr = strdup( buf );
		size = byteRead;

		if ( close( procs.fd[R] ) == ERROR ) throwSysErr( "close", 500 );

		// std::clog << "[read data from CGI]\n" << buf << ";\n";
		// oss << buf;
		// std::clog << "[copied data from buf]\n" << oss.str() << ";\n";
	// }
	
	// while ( read( procs.fd[R], &buf, 1024 ) != NONE ) {
	// 	oss << buf;
	// 	if ( oss.fail() ) throwSysErr( "read", 500 );
	// }
	
	// std::clog << "_read start dupStreamBuffer\n";
	// *bufptr = dupStreamBuffer( oss, size );
}


void
CGI::_argvBuild( vec_cstr_t& argv, const str_t& cmd, const str_t& arg ) {
	argv.push_back( const_cast<char*>( cmd.c_str() ) );
	argv.push_back( const_cast<char*>( arg.c_str() ) );
}


stat_t
CGI::_autoindex( const Request& rqst, const process_t& procs ) {
	// std::cerr << "child > _autoindex\n";
	if ( chdir( argv[2] ) != ERROR ) {
		// argv[2] = NULL;
		// std::cerr << "child > _execve\n";
		return _execve( procs, argv );
	}
	return EXIT_FAILURE;
}


process_s::process_s( void ) {
	pid		= NONE;
	fd[R]	= NONE;
	fd[W]	= NONE;
	stat	= NONE;
}