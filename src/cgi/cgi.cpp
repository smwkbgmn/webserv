#include "CGI.hpp"

void
CGI::GET( const Request& rqst, char** bufptr, size_t& size ) {
	process_t	procs;
	vec_cstr_t	argv;

	logfile.fs << "the CGI - GET invoked\n";

	// autoindex
	if ( *rqst.line().uri.rbegin() == '/' ) {
		logfile.fs << "_argBuild start\n";
		_argvBuild( argv, "/usr/bin/php", HTTP::http.fileAtidx );
		logfile.fs << "_argBuild finish\n";
		
		logfile.fs << "_detach start\n";
		if ( _detach ( procs, &_execute, argv ) == SUCCESS ) {
			logfile.fs << "_detach finish\n";
			// _redirect( procs );
			logfile.fs << "_wait start\n";
			_wait( procs );
			logfile.fs << "_wait finish\n";
			
			if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
				throw errstat_t( 500 );

			logfile.fs << "_read start\n";
			_read( procs, bufptr, size );
			logfile.fs << "_read finish\n";
		}
		else
			throwSysErr( "execve", 500 );
	}

	logfile.fs << "the CGI - GET has finished\n";

}

void
CGI::POST( const Request& rqst, char** bufptr, size_t& size ) {
	( void )rqst;
	( void )bufptr;
	( void )size;
}

void
CGI::_argvBuild( vec_cstr_t& argv, const str_t& cmd, const str_t& arg ) {
	argv.push_back( const_cast<char*>( cmd.c_str() ) );
	argv.push_back( const_cast<char*>( arg.c_str() ) );
}

stat_t
CGI::_detach( process_t& procs, fn_t execute, vec_cstr_t& argv ) {
	if ( pipe( procs.fd ) == ERROR || ( procs.pid = fork() ) == ERROR )
		throw errstat_t( 500 );
	
	if ( !procs.pid ) return execute( procs, argv.data() );
	else return SUCCESS;
}

void
CGI::_redirect( const process_t& procs ) {
	if ( dup2( procs.fd[W], STDOUT_FILENO ) == ERROR  ||
		close( procs.fd[R] ) == ERROR || close( procs.fd[W] ) == ERROR )
		throw errstat_t( 500 );

	// if ( ( procs.pid && dup2( procs.fd[R], STDIN_FILENO ) == ERROR ) ||
	// 	( !procs.pid && dup2( procs.fd[W], STDOUT_FILENO ) == ERROR ) ||
	// 	( close( procs.fd[R] ) == ERROR || close( procs.fd[W] ) == ERROR ) )
	// 	throw errstat_t( 500 );
}

stat_t
CGI::_execute( const process_t& procs, char* argv[] ) {
	try { _redirect( procs ); return execve( *argv, argv, NULL ); }
	catch ( errstat_t& erstat ) { return EXIT_FAILURE; }
}

void
CGI::_wait( process_t& procs ) {
	if ( waitpid( procs.pid, &procs.stat, NONE ) == ERROR )
		throw errstat_t( 500 );
}
 
void
CGI::_read( process_t& procs, char** bufptr, size_t& size ) {
	osstream_t	oss;
	char		buf[1024];

	while ( read( procs.fd[R], &buf, 1024 ) )
		oss << buf;
	
	*bufptr = dupStreamBuffer( oss, size );
}

process_s::process_s( void ) {
	pid		= NONE;
	fd[R]	= NONE;
	fd[W]	= NONE;
	stat	= NONE;
}