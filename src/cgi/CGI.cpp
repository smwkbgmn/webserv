#include "CGI.hpp"

// autoindexing (GET/.php), RPN calculator (POST/.exe), sorting (POST/.cpp)

void
CGI::proceed( const Request& rqst, osstream_t& oss ) {
	process_t	procs;
	fnptr_t		act = NULL;
	
	if ( *rqst.line().uri.rbegin() == '/' ) act = &_autoindex;
	else act = &_script;

	if ( _detach( rqst, procs, act ) == SUCCESS ) {
		_write( procs, rqst );
		_wait( procs );
		_read( procs, oss );

		if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
			throw errstat_t( 500, "the CGI fail to exit as SUCCESS" );
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

stat_t
CGI::_script( const Request& rqst, const process_t& procs ) {
	// str_t		script_path = "html" + rqst.line().uri;
	str_t		script_path = rqst.line().uri;
	vec_cstr_t	argv_c;
	vec_cstr_t	env_c;

	argv_c.push_back( const_cast<char*>( script_path.c_str() ) );
	argv_c.push_back( NULL );

	env_c.push_back( const_cast<char*>( "REQUEST_METHOD=POST" ) );
	env_c.push_back( const_cast<char*>( "SERVER_PROTOCOL=HTTP/1.1" ) );
	env_c.push_back( const_cast<char*>( "SCRIPT_FILENAME=upload.cgi" ) );
	env_c.push_back( const_cast<char*>( "SERVER_PORT=8080" ) );
	env_c.push_back( const_cast<char*>( "SERVER_NAME=webserv" ) );

	osstream_t	oss;
	oss << "CONTENT_LENGTH=";
	oss << rqst.header().content_length;
	env_c.push_back( const_cast<char*>( oss.str().c_str() ) );

	env_c.push_back( const_cast<char*>( "CONTENT_TYPE=multipart/form-data" ) );
	env_c.push_back( NULL );

	return _execve( procs, argv_c.data(), env_c.data() );
}

stat_t
CGI::_autoindex( const Request& rqst, const process_t& procs ) {
	vec_str_t	argv;
	vec_cstr_t	argv_c;
	vec_cstr_t	env_c;

	argv.push_back( binPHP );
	argv.push_back( HTTP::http.fileAtidx );

	for ( vec_str_t::const_iterator iter = argv.begin(); iter != argv.end(); ++iter )
		argv_c.push_back( const_cast<char*>( iter->c_str() ) );
	argv_c.push_back( NULL );

	str_t	path_info = varPATH_INFO + rqst.line().uri;
	env_c.push_back( const_cast<char*>( path_info.c_str() ) ); 
	env_c.push_back( NULL );
	
	return _execve( procs, argv_c.data(), env_c.data() );
}


// Be aware that functions under _detach are running on subprocess
bool
CGI::_redirect( const process_t& procs ) {
	if ( dup2( procs.fd[R], STDIN_FILENO ) == ERROR ||
		dup2( procs.fd[W], STDOUT_FILENO ) == ERROR ||
		close( procs.fd[R] ) == ERROR ||
		close( procs.fd[W] ) == ERROR )
		return FALSE;
	return TRUE;	
}

stat_t
CGI::_execve( const process_t& procs, char* argv[], char* env[] ) {
	clog( "CGI - argv" );
	for ( size_t ptr = 0; argv[ptr]; ++ptr )
		std::clog << argv[ptr] << "\n";

	clog( "CGI - env" );
	for ( size_t ptr = 0; env[ptr]; ++ptr )
		std::clog << env[ptr] << "\n";

	if ( _redirect( procs ) )
		return execve( *argv, argv, env );
	return EXIT_FAILURE;
}


void
CGI::_write( const process_t& procs, const Request& rqst ) {
	if ( rqst.line().method == POST ) {
		if ( write( procs.fd[W], rqst.body(), rqst.header().content_length ) == ERROR)
			throwSysErr( "write", 500 );
	}
	close( procs.fd[W] );
}

void
CGI::_wait( process_t& procs ) {
	// Should be replaced the NONE with WNOHANG after restruct the flow
	if ( waitpid( procs.pid, &procs.stat, NONE ) == ERROR )
		throwSysErr( "wait", 500 );
}
 
void
CGI::_read( process_t& procs, osstream_t& oss ) {
	char	buf[1500]	= { NONE };
	ssize_t	bytes		= 0;

	if ( ( bytes = read( procs.fd[R], buf, 1500 ) ) == ERROR )
		throwSysErr( "read", 500 );

	if ( bytes == ERROR )
		throwSysErr( "read", 500 );

	oss << "HTTP/1.1 200 OK\r\n";
    oss << "Content-Type: text/html\r\n";
	oss << "Content-Length: " << bytes << CRLF;
	oss << CRLF;
	oss << str_t( buf );

	close( procs.fd[R] );
}

process_s::process_s( void ) {
	pid		= NONE;
	fd[R]	= NONE;
	fd[W]	= NONE;
	stat	= NONE;
}