#include "CGI.hpp"

map_str_path_t	CGI::script_bin;
map_uint_str_t	CGI::environ_list;

/*
	.cgi, .exe and files in configured cgi-bin directory
	-> execute directly

	other else .perl, .php, .py.. so on
	-> /usr/bin/perl..
*/

/* METHOD - init: assign keys for running CGI */
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
	File	fileEnv( fileEnviron, READ );
	str_t	key;
	
	for ( uint_t keyidx = 0; std::getline( fileEnv.fs, key ); ++keyidx )
		environ_list.insert( std::make_pair<uint_t, str_t>( keyidx, key ) );
}

/* METHOD - proceed: get outsourcing data */
void
CGI::proceed( const Request& rqst, process_t& procs, osstream_t& oss ) {
	osstream_t source;
	log( "CGI\t: proceed" );

	if ( _detach( rqst, procs ) == SUCCESS ) {
		_write( procs, rqst );
		_wait( procs );
		_read( procs, source );
		_build( source, oss );

		if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
			throw errstat_t( 500, "the CGI fail to exit as SUCCESS" );
	}
	else throwSysErr( "_detach", 500 );
}

stat_t
CGI::_detach( const Request& rqst, process_t& procs ) {
	if ( pipe( procs.fd ) == ERROR || ( procs.pid = fork() ) == ERROR )
		return EXIT_FAILURE;

	if ( !procs.pid ) {
		_buildEnviron( rqst, procs );

		return _execve( procs );
	}
	return SUCCESS;
}

/* PARENT */
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
CGI::_read( process_t& procs, osstream_t& source ) {
	c_buffer_t	buf;
	
	while ( ( buf.read = read( procs.fd[R], buf.ptr, SIZE_BUF ) ) > 0 ) {
		source.write( buf.ptr, buf.read );
		buf.total += buf.read;
	}
	if ( buf.read == ERROR ) throwSysErr( "read", 500 );
	
	close( procs.fd[R] );
}

void
CGI::_build( osstream_t& source, osstream_t& oss ) {
	size_t size = source.str().length();

	if ( !size )
		oss << "HTTP/1.1 204" << SP << HTTP::key.status.at( 204 ) << CRLF;

	else {
		oss << "HTTP/1.1 200 OK" << CRLF;

		if ( source.str().find( "Content-Type" ) == str_t::npos )
			oss << "Content-Type: text/plain" << CRLF;

		if ( source.str().find( "Content-Length" ) == str_t::npos ) {
			size_t pos_header_end = source.str().find( MSG_END );

			if ( pos_header_end != str_t::npos )
				size -= pos_header_end - 4;
			oss << "Content-Length: " << size << CRLF;
		}
		oss << source.str();
	}
}

/* CHILD */
void
CGI::_buildEnviron( const Request& rqst, process_t& procs ) {
	for ( uint_t idx = 0; _buildEnvironVar( rqst, procs, idx ); ++idx );
}

bool
CGI::_buildEnvironVar( const Request& rqst, process_t& procs, uint_t idx ) {
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
			case CONTENT_TYPE		: oss << rqst.header().content_type; break;
			case PATH_INFO			: oss << rqst.line().uri.substr( rqst.config().root.length() + 1 ); break;
			case PATH_TRANSLATED	: oss << rqst.line().uri; break;
			case QUERY_STRING		: oss << rqst.line().query; break;
		}
		procs.env.push_back( oss.str() );
		return TRUE;
	}
	catch ( exception_t& exc ) { return FALSE; }
}

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
	vec_cstr_t	argv_c;
	vec_cstr_t	env_c;

	_assignVectorChar( argv_c, procs.argv );
	_assignVectorChar( env_c, procs.env );

	if ( _redirect( procs ) )
		return execve( argv_c[0], argv_c.data(), env_c.data() ); 
	return EXIT_FAILURE;
}

void
CGI::_assignVectorChar( vec_cstr_t& vec_char, const vec_str_t& vec_str ) {
	for ( vec_str_t::const_iterator iter = vec_str.begin(); iter != vec_str.end(); ++iter )
		vec_char.push_back( const_cast<char*>( iter->c_str() ) );
	vec_char.push_back( NULL );
}

/* STRUCT */
c_buffer_s::c_buffer_s( void ) {
	total	= 0;
	read	= 0;
}

process_s::process_s( void ) { reset();	}

void
process_s::reset( void ) {
	pid		= NONE;
	stat	= NONE;
	fd[R]	= NONE;
	fd[W]	= NONE;	
}