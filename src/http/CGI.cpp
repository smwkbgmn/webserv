#include "CGI.hpp"

map_str_path_t	CGI::script_bin;
map_uint_str_t	CGI::environ_list;

/*k
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
	File	file_env( file_environ, READ );
	str_t	key;
	
	for ( uint_t keyidx = 0; std::getline( file_env.fs, key ); ++keyidx )
		environ_list.insert( std::make_pair( keyidx, key ) );
}

/* METHOD - proceed: get outsourcing data */
void
CGI::proceed( const Request& rqst, process_t& procs, sstream_t& out_body ) {
	log( "CGI\t: proceed" );

	if ( _detach( rqst, procs ) == SUCCESS ) {
		// _write( procs, rqst );
		// _wait( procs );

		// sstream_t data;
		// _build( data, out_body, _read( procs, data ) );

		// if ( WEXITSTATUS( procs.stat ) != EXIT_SUCCESS )
		// 	throw errstat_t( 500, "the CGI fail to exit as SUCCESS" );
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
// void
// CGI::_write( const process_t& procs, const Request& rqst ) {
// 	if ( rqst.line().method == POST &&
// 		write( procs.fd[W], rqst.body().str().c_str(), rqst.header().content_length ) == ERROR ) 
// 			throwSysErr( "write", 500 );
// 	close( procs.fd[W] );
// }

void
CGI::writeTo( const process_t& procs, const char* in_body, const size_t& size ) {
	if ( write( procs.fd[W], in_body, size ) == ERROR )
		throwSysErr( "write", 500 );
}

void
// CGI::_wait( process_t& procs ) {
CGI::wait( process_t& procs ) {
	// Should be replaced the NONE with WNOHANG after restruct the flow
	if ( waitpid( procs.pid, &procs.stat, NONE ) == ERROR )
		throwSysErr( "wait", 500 );
}

// size_t
// CGI::_read( process_t& procs, sstream_t& data ) {
void
CGI::readFrom( const process_t& procs, sstream_t& out_body) {
	c_buffer_t	buf;
	
	while ( ( buf.read = read( procs.fd[R], buf.ptr, SIZE_BUF ) ) > 0 ) {
		out_body.write( buf.ptr, buf.read );
		buf.total += buf.read;
	}
	if ( buf.read == ERROR ) throwSysErr( "read", 500 );
	
	close( procs.fd[R] );
}

void
// CGI::_build( sstream_t& data, sstream_t& out_body, size_t size ) {
CGI::build( msg_buffer_t& out ) {
	_buildLine( out );
	if ( out.body.str().size() )
		_buildHeader( out );
}

void
// CGI::_buildLine( sstream_t& data, const size_t& size ) {
CGI::_buildLine( msg_buffer_t& out ) {
	out.msg << 
	HTTP::http.signature << '/' << HTTP::http.version.at( VERSION_11 ) << SP;

	if ( !out.body.str().size() ) out.msg << "204" << SP << HTTP::key.status.at( 204 ) << CRLF;
	else out.msg << "200" << SP << HTTP::key.status.at( 200 );
	
	out.msg << CRLF;
}

void
// CGI::_buildHeader( const sstream_t& data, sstream_t& out_body, size_t& size ) {
CGI::_buildHeader( msg_buffer_t& out ) {
	size_t pos_header_end = out.body.str().find( MSG_END );

	if ( found( pos_header_end ) ) {
		out.msg << out.body.str().substr( 0, pos_header_end + 4 );
		out.body.ignore( pos_header_end + 4 );
	}

	if ( !found( pos_header_end ) || ( found( pos_header_end ) &&
		!found( out.msg.str().find( HTTP::key.header_out.at( OUT_CONTENT_TYPE ) ) ) ) ) {
		out.msg <<
		HTTP::key.header_out.at( OUT_CONTENT_TYPE ) << ':' << SP <<
		HTTP::key.mime.at( "txt" ) << CRLF;
	}
		 
	if ( !found( pos_header_end ) || ( found( pos_header_end ) &&
		!found( out.msg.str().find( HTTP::key.header_out.at( OUT_CONTENT_LEN ) ) ) ) ) {
		out.msg << 
		HTTP::key.header_out.at( OUT_TRANSFER_ENC ) << ':' << SP <<
		HTTP::http.encoding.at( TE_CHUNKED ) << CRLF;

		out.chunked = TRUE;
	}
	
	out.msg << CRLF;
	
	// if ( size ) {
	// 	size_t	pos_header_end	= data.str().find( MSG_END );

	// 	if ( !found( data.str().find( HTTP::key.header_out.at( OUT_CONTENT_TYPE ) ) ) )
	// 		out_body <<
	// 		HTTP::key.header_out.at( OUT_CONTENT_TYPE ) << ':' << SP <<
	// 		HTTP::key.mime.at( "txt" ) << CRLF;

	// 	if ( !found( data.str().find( HTTP::key.header_out.at( OUT_CONTENT_LEN ) ) ) ) {
	// 		if ( found( pos_header_end ) )
	// 			size -= pos_header_end - 4;

	// 		out_body <<
	// 		HTTP::key.header_out.at( OUT_CONTENT_LEN ) << ':' << SP <<
	// 		size << CRLF;
	// 	}

	// 	if ( !found( pos_header_end ) )
	// 		out_body << CRLF;

	// 	out_body << data.str();
	// }
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
			case SERVER_NAME		: oss << rqst.config().name; break;
			case SERVER_PORT		: oss << rqst.config().listen; break;
			case SERVER_PROTOCOL	: oss << HTTP::http.signature << '/' << HTTP::http.version.at( VERSION_11 ); break;
			case REMOTE_ADDR		: break;
			case REMOTE_HOST		: break;
			case GATEWAY_INTERFACE	: break;
			case REQUEST_METHOD		: oss << str_method[rqst.line().method]; break;
			case SCRIPT_NAME		: oss << rqst.line().uri.substr( rqst.line().uri.rfind( '/' ) + 1 ); break;

			case CONTENT_LENGTH		:
				if ( rqst.line().method == POST && rqst.header().transfer_encoding != TE_CHUNKED )
					oss << rqst.header().content_length; break;

			case CONTENT_TYPE		:
				if ( rqst.line().method == POST )
					oss << rqst.header().content_type; break;

			case PATH_INFO			: oss << rqst.line().uri.substr( rqst.location().root.length() + 1 ); break;
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
