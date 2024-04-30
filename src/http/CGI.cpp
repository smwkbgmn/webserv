#include "CGI.hpp"

/*
	.cgi, .exe and files in configured cgi-bin directory
	-> execute directly

	other else .perl, .php, .py.. so on
	-> /usr/bin/perl..
*/

void
CGI::init( void ) {
	_bin.insert( std::make_pair<str_t, path_t>( ".php", "/usr/bin/php" ) );
	_bin.insert( std::make_pair<str_t, path_t>( ".perl", "/usr/bin/perl" ) );
	_bin.insert( std::make_pair<str_t, path_t>( ".py", "/usr/bin/python" ) );
}

void
CGI::proceed( const Request& rqst, process_t& procs, osstream_t& oss ) {
	if ( _detach( rqst, oss, procs ) == SUCCESS ) {
		_write( procs, rqst );
		_wait( procs );
		_read( procs, oss );

		if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
			throw errstat_t( 500, "the CGI fail to exit as SUCCESS" );
	}
	else throwSysErr( "_detach", 500 );
}

stat_t
CGI::_detach( const Request& rqst, osstream_t& oss, process_t& procs ) {
	if ( pipe( procs.fd ) != ERROR || ( procs.pid = fork() ) != ERROR )
		return EXIT_FAILURE;
	
	if ( !procs.pid ) {
		if ( write( procs.fd[W], "HTTP/1.1 200 OK\r\n", 17 ) == ERROR ) return EXIT_FAILURE;
		return procs.act( rqst, procs );
	}
	return SUCCESS;
}

stat_t
CGI::_script( const Request& rqst, process_t& procs ) {
	str_t		script_path = rqst.line().uri;
	vec_cstr_t	argv_c;
	vec_cstr_t	env_c;

	assignEnv( rqst, procs.env );

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

	if ( write( procs.fd[W], "Content-Type: text/html\r\n", 25 ) == ERROR )
		return EXIT_FAILURE;

	return _execve( procs, argv_c.data(), env_c.data() );
}

stat_t
CGI::_execute( const Request& rqst, process_t& procs ) {

}

stat_t
CGI::_autoindex( const Request& rqst, process_t& procs ) {
	vec_str_t	argv;
	vec_cstr_t	argv_c;
	vec_cstr_t	env_c;

	argv.push_back( binPHP );
	argv.push_back( HTTP::http.fileAtidx );

	for ( vec_str_t::const_iterator iter = argv.begin(); iter != argv.end(); ++iter )
		argv_c.push_back( const_cast<char*>( iter->c_str() ) );
	argv_c.push_back( NULL );

	str_t	path_info = "PATH_INFO=" + rqst.line().uri;
	env_c.push_back( const_cast<char*>( path_info.c_str() ) ); 
	env_c.push_back( NULL );

	if ( write( procs.fd[W], "Content-Type: text/html\r\n", 25 ) == ERROR )
		return EXIT_FAILURE;
	
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
	clog( "CGI\t: argv" );
	for ( size_t ptr = 0; argv[ptr]; ++ptr )
		std::clog << argv[ptr] << "\n";

	clog( "CGI\t: env" );
	for ( size_t ptr = 0; env[ptr]; ++ptr )
		std::clog << env[ptr] << "\n";

	if ( _redirect( procs ) )
		return execve( *argv, argv, env );
	return EXIT_FAILURE;
}


void
CGI::_write( const process_t& procs, const Request& rqst ) {
	if ( rqst.line().method == POST &&
		write( procs.fd[W], rqst.body(), rqst.header().content_length ) == ERROR ) 
			throwSysErr( "write", 500 );
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

	oss << "HTTP/1.1 200 OK" << CRLF;
	oss << "Content-Length: " << bytes << CRLF;
	// oss << "Content-Length: " << bytes << CRLF << CRLF;
	oss << str_t( buf );

	close( procs.fd[R] );
}

/* METHOD - assignEnv: build ENVs for CGI script */
void
CGI::assignEnv( const Request& rqst, vec_cstr_t& env ) {
	
}

/* STRUCT */
process_s::process_s( void ) { reset();	}

void
process_s::reset( void ) {
	act		= EXECUTABLE;

	pid		= NONE;
	stat	= NONE;
	fd[R]	= NONE;
	fd[W]	= NONE;	
}