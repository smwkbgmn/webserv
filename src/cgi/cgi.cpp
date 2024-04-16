#include "CGI.hpp"

// autoindexing (GET/.php), RPN calculator (POST/.exe), sorting (POST/.cpp)

void
CGI::proceed( const Request& rqst, osstream_t& oss ) {
	process_t	procs;
	fnptr_t		act = NULL;
	
	if ( *rqst.line().uri.rbegin() == '/' )
		act = &_autoindex;

	if ( _detach( rqst, procs, act ) == SUCCESS ) {
		_wait( procs );
		_read( procs, oss );
		
		if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
			throw errstat_t( 500 );
	}
	else throwSysErr( "execve", 500 );
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
		close( procs.fd[R] ) == ERROR ||
		close( procs.fd[W] ) == ERROR )
		return FALSE;
	return TRUE;	
}

stat_t
CGI::_execve( const process_t& procs, char* argv[], char* env[] ) {
	std::clog << "CGI - received argv\n";
	for ( size_t ptr = 0; argv[ptr]; ++ptr )
		std::clog << argv[ptr] << "\n";

	std::clog << "CGI - received env\n";
	for ( size_t ptr = 0; env[ptr]; ++ptr )
		std::clog << env[ptr] << "\n";

	if ( _redirect( procs ) )
		return execve( *argv, argv, env );
	return EXIT_FAILURE;
}

void
CGI::_wait( process_t& procs ) {
	// Should be replaced the NONE with WNOHANG after restruct the flow
	if ( waitpid( procs.pid, &procs.stat, NONE ) == ERROR )
		throwSysErr( "wait", 500 );
}
 
void
CGI::_read( process_t& procs, osstream_t& oss ) {
	char	buf[1024];
	ssize_t	bytes = 0;

	if ( ( bytes = read(procs.fd[R], buf, 1024 ) ) == ERROR )
		throwSysErr( "read", 500 );
	
	oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: text/html\r\n";
	oss << "Content-Length: " << bytes << CRLF;
	oss << CRLF;
	oss << buf;
}

stat_t
CGI::_autoindex( const Request& rqst, const process_t& procs ) {
	vec_str_t	argv;
	vec_cstr_t	argv_c;
	vec_cstr_t	env_c;

	argv.push_back( binPHP );
	argv.push_back( rqst.config().root + HTTP::http.fileAtidx );

	for ( vec_str_t::const_iterator iter = argv.begin(); iter != argv.end(); ++iter )
		argv_c.push_back( const_cast<char*>( iter->c_str() ) );
	argv_c.push_back( NULL );

	str_t	path_info = varPATH_INFO + rqst.config().root + rqst.line().uri;
	// str_t	path_info = varPATH_TRANSLATED + rqst.config().root + rqst.line().uri;
	env_c.push_back( const_cast<char*>( path_info.c_str() ) ); 
	env_c.push_back( NULL );
	
	return _execve( procs, argv_c.data(), env_c.data() );
}

process_s::process_s( void ) {
	pid		= NONE;
	fd[R]	= NONE;
	fd[W]	= NONE;
	stat	= NONE;
}