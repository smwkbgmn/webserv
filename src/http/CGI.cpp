#include "CGI.hpp"

map_str_path_t	CGI::script_bin;
map_uint_str_t	CGI::environ_list;

/*
	.cgi, .exe and files in configured cgi-bin directory
	-> execute directly

	other else .perl, .php, .py.. so on
	-> /usr/bin/perl..
*/

void
CGI::init( void ) {
	_assignScriptBin();
	_assignEnvironList();
}

void
CGI::_assignScriptBin( void ) {
	script_bin.insert( std::make_pair<str_t, path_t>( ".php", "/usr/bin/php" ) );
	script_bin.insert( std::make_pair<str_t, path_t>( ".pl", "/usr/bin/perl" ) );
	script_bin.insert( std::make_pair<str_t, path_t>( ".py", "/usr/bin/python" ) );
}

void
CGI::_assignEnvironList( void ) {
	File	fileEnv( fileEnviron, R );
	str_t	key;
	
	for ( uint_t keyidx = 0; std::getline( fileEnv.fs, key ); ++keyidx )
		environ_list.insert( std::make_pair<uint_t, str_t>( keyidx, key ) );

	// while ( std::getline( fileEnv.fs, key ) )
	// 	environ_list.push_back( key );
}

void
CGI::proceed( const Request& rqst, process_t& procs, osstream_t& oss ) {
	if ( _detach( rqst, procs ) == SUCCESS ) {
		_write( procs, rqst );
		_wait( procs );
		_read( procs, oss );

		if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
			throw errstat_t( 500, "the CGI fail to exit as SUCCESS" );
	}
	else throwSysErr( "_detach", 500 );
}

stat_t
CGI::_detach( const Request& rqst, process_t& procs ) {
	std::clog << "_detach\n";

	if ( pipe( procs.fd ) == ERROR || ( procs.pid = fork() ) == ERROR )
		return EXIT_FAILURE;

	if ( !procs.pid ) {
		if ( write( procs.fd[W], "HTTP/1.1 200 OK\r\n", 17 ) == ERROR ||
			write( procs.fd[W], "Content-Type: text/html\r\n", 25 ) == ERROR )
			return EXIT_FAILURE;

		_buildEnviron( rqst, procs.env );

		if ( procs.act == AUTOINDEX ) return _autoindex( procs );
		// else return _script( rqst, procs );
		else return _execute( rqst, procs );
	}
	return SUCCESS;
}

stat_t
CGI::_execute( const Request& rqst, process_t& procs ) {
	std::clog << "_execute\n";
	procs.argv.push_back( const_cast<char*>( rqst.line().uri.c_str() ) );
	procs.argv.push_back( NULL );

	return _execve( procs );
}

stat_t
CGI::_autoindex( process_t& procs ) {
	std::clog << "_autoindex\n";
	// vec_str_t	argv;
	// vec_cstr_t	argv_c;
	// vec_cstr_t	env_c;

	// argv.push_back( "/usr/bin/php" );
	// argv.push_back( HTTP::http.fileAtidx );

	// for ( vec_str_t::const_iterator iter = argv.begin(); iter != argv.end(); ++iter )
	// 	argv_c.push_back( const_cast<char*>( iter->c_str() ) );
	// argv_c.push_back( NULL );

	// str_t	path_info = "PATH_INFO=" + rqst.line().uri;
	// env_c.push_back( const_cast<char*>( path_info.c_str() ) ); 
	// env_c.push_back( NULL );

	procs.argv.push_back( const_cast<char*>( HTTP::http.fileAtidx.c_str() ) );
	procs.argv.push_back( NULL );

	return _execve( procs );
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
CGI::_execve( const process_t& procs ) {
	clog( "CGI\t: argv" );
	for ( size_t ptr = 0; procs.argv[ptr]; ++ptr )
		std::clog << str_t( procs.argv[ptr] ) << std::endl;

	clog( "CGI\t: env" );
	for ( size_t ptr = 0; procs.env[ptr]; ++ptr )
		std::clog << str_t( procs.env[ptr] ) << std::endl;

	if ( _redirect( procs ) )
		return execve( procs.argv[0], procs.argv.data(), procs.env.data() ); return EXIT_FAILURE;
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

	oss << "HTTP/1.1 200 OK" << CRLF;
	oss << "Content-Length: " << bytes << CRLF;
	oss << str_t( buf );

	close( procs.fd[R] );
}

/* METHOD - assignEnv: build ENVs for CGI script */
void
CGI::_buildEnviron( const Request& rqst, vec_cstr_t& env ) {
	std::clog << "_buildEnvrion\n";
	for ( uint_t idx = 0; _buildEnvironVar( rqst, env, idx ); ++idx );
	env.push_back( NULL );
}

bool
CGI::_buildEnvironVar( const Request& rqst, vec_cstr_t& env, uint_t idx ) {
	clog( "_buildingEnvironVar" );
	try {
		osstream_t	oss;
		oss << environ_list.at( idx ) << '=';

		switch ( idx ) {
			case SERVER_NAME		: oss << "webserv"; break;
			case SERVER_PORT		: oss << "8080"; break;
			case SERVER_PROTOCOL	: oss << "HTTP/1.1"; break;
			case REMOTE_ADDR		: break;
			case REMOTE_HOST		: break;
			case GATEWAY_INTERFACE	: break;
			case REQUEST_METHOD		: oss << strMethod[rqst.line().method]; break;
			case SCRIPT_NAME		: oss << rqst.line().uri.substr( rqst.line().uri.rfind( '/' ) + 1 ); break;
			case CONTENT_LENGTH		: if ( rqst.line().method == POST ) oss << rqst.header().content_length; break;
			case CONTENT_TYPE		: if ( rqst.line().method == POST ) oss << rqst.header().content_type; break;
			case PATH_INFO			: oss << rqst.line().uri.substr( rqst.config().root.length() + 1 ); break;
			case PATH_TRANSLATED	: oss << rqst.line().uri; break;
			case QUERY_STRING		: break;
		}
		clog( oss.str().c_str() );
		env.push_back( strdup( const_cast<char*>( oss.str().c_str() ) ) );
		clog( env.at( idx ) );

		return TRUE;
	}
	catch ( exception_t& exc ) { return FALSE; }
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