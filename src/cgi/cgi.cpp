#include "CGI.hpp"

// autoindexing (GET/.php), RPN calculator (POST/.exe), sorting (POST/.cpp)

void
CGI::proceed( const Request& rqst, osstream_t& oss ) {
	process_t	procs;
	fnptr_t		act = NULL;
	
	clog( "CGI - proceed" );

	if ( *rqst.line().uri.rbegin() == '/' ) act = &_autoindex;
	else act = &_script;

	if ( _detach( rqst, procs, act ) == SUCCESS ) {
		clog( "_write" );
		_write( procs, rqst );

		clog( "_wait" );
		_wait( procs );

		clog( "_read" );
		close( procs.fd[W] );
		_read( procs, oss );
		
		if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
			throw errstat_t( 500 );
	}
	else throwSysErr( "execve", 500 );
}

void
CGI::_write( const process_t& procs, const Request& rqst ) {
	if ( rqst.line().method == POST ) {
		// const char* data = rqst.client()
		ssize_t sizeMsg = strlen( rqst.client().buffer() );

		// Should the size of buf be checked
		// for ( ssize_t pos = 0; pos < sizeMsg; pos = write( procs.fd[W], rqst.client().buf, 1024 ) )
		// 	if ( pos == ERROR )
		// 		throwSysErr( "write", 500 );
	
		write( procs.fd[W], rqst.client().buffer(), sizeMsg );


		// clog( "CGI - written data" );
		// std::clog << rqst.client().buffer();

		// for ( int cnt = 0; cnt < 3; ++cnt ) {
		// 	char	buf[100];
		// 	ssize_t	byte	= read( rqst.client().socket(), buf, 100 );

		// 	if ( byte == ERROR )
		// 		throwSysErr( "read", 500 );

		// 	buf[byte] = NONE;
		// 	write( procs.fd[W], buf, byte );


			// clog( "CGI - additional read data" );
			// std::clog << buf;
		// }


			

		// char	buf[20000];
		// ssize_t	bytes;
		// size_t	total = 0;

		// while ( total < rqst.header().content_length ) {
		// 	sizeMsg = 0;
		// 	while ( ( bytes = read( rqst.client().socket(), &buf[sizeMsg], 1 ) ) > 0 && sizeMsg < 20000 ) {
		// 		sizeMsg += bytes;
		// 		// std::clog << sizeMsg << "\n";
		// 	}

		
		// 	buf[sizeMsg] = '\0';
		// 	clog( "CGI - upload data size " );
		// 	std::clog << sizeMsg << '/' << rqst.header().content_length << std::endl;

		// 	write( procs.fd[W], buf, sizeMsg );

		// 	// clog( "CGI - upload body" );
		// 	// std::clog << buf << std::endl;
			
		// 	total += sizeMsg;
		// }




		// char	buf[20000];
		// ssize_t	bytes = read( rqst.client().socket(), buf, 20000 );
		// buf[bytes] = NONE;

		// clog( "CGI - received data size" );
		// std::clog << bytes << std::endl;

		// write( procs.fd[W], buf, bytes );

		// clog( "CGI - read data" );
		// std::clog << buf;
			
		// dup2( rqst.client().socket(), procs.fd[W] );
		// dup2( rqst.client().socket(), STDOUT_FILENO );
	}
	close( procs.fd[W] );
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

	clog ( "CGI - env" );
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
	char	buf[10000];
	ssize_t	bytes = 0;

	// if ( ( bytes = read(procs.fd[R], buf, 1024 ) ) == ERROR )
	// 	throwSysErr( "read", 500 );

	while ( ( bytes = read( procs.fd[R], buf, 10000 ) ) > 0 )
		if ( bytes == ERROR )
			throwSysErr( "read", 500 );

	close( procs.fd[R] );
	
	// oss << "HTTP/1.1 200 OK\r\n";
    // oss << "Content-Type: text/html\r\n";
	// oss << "Content-Length: " << bytes << CRLF;
	// oss << CRLF;
	// oss << buf << '\0';
	clog ( "CGI - produced data\n" + oss.str() );
}

stat_t
CGI::_script( const Request& rqst, const process_t& procs ) {
	str_t		script_path = "html" + rqst.line().uri;
	vec_cstr_t	argv_c;
	vec_cstr_t	env_c;

	argv_c.push_back( const_cast<char*>( script_path.c_str() ) );
	argv_c.push_back( NULL );

	env_c.push_back( const_cast<char*>( "REQUEST_METHOD=POST" ) );
	env_c.push_back( const_cast<char*>( "SERVER_PROTOCOL=HTTP/1.1" ) );
	env_c.push_back( const_cast<char*>( "SCRIPT_FILENAME=upload.cgi" ) );
	env_c.push_back( const_cast<char*>( "SERVER_PORT=8080" ) );
	env_c.push_back( const_cast<char*>( "SERVER_NAME=webserv" ) );
	// env_c.push_back( const_cast<char*>( "CONTENT_LENGTH=123" ) );
	// env_c.push_back( const_cast<char*>( "CONTENT_LENGTH=6607" ) );
	osstream_t	oss;
	oss << "CONTENT_LENGTH=";
	oss << rqst.header().content_length;
	env_c.push_back( const_cast<char*>( oss.str().c_str() ) );
	// env_c.push_back( const_cast<char*>( "CONTENT_LENGTH=1024" ) );
	env_c.push_back( const_cast<char*>( "CONTENT_TYPE=multipart/form-data" ) );
	// env_c.push_back( const_cast<char*>( "CONTENT_TYPE=text/plain" ) );
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