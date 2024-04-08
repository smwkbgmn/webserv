#include "HTTP.hpp"

http_t			HTTP::http;
config_t		HTTP::config;
key_t			HTTP::key;

/* METHOD - init: load keys */
void
HTTP::init( void ) {
	http.server			= "ft_webserv";
	http.signature		= "HTTP";
	config.typeUnrecog	= "text/plain";
	config.nameNotFound	= "/404_not_found.html";
	config.dirRoot		= "./html";
	
	_assignHeader();
	_assignStatus();
	_assignMime();
	_assignVec( http.version, strVersion, CNT_VERSION );
	_assignVec( http.method, strMethod, CNT_METHOD );
}

void
HTTP::_assignHeader( void ) {
	str_t	header;

	File fileIn( nameHeaderIn, R );
	while ( std::getline( fileIn.fs, header ) )
		key.header_in.push_back( header );

	File fileOut( nameHeaderOut, R );
	while ( std::getline( fileOut.fs, header ) )
		key.header_out.push_back( header );
}

void
HTTP::_assignStatus( void ) {
	File file( nameStatus, R );

	while ( !file.fs.eof() ) {
		uint_t	code;
		str_t	reason;

		file.fs >> code;
		file.fs.get();
		std::getline( file.fs, reason );

		key.status.insert( std::make_pair( code, reason ) );
	}
}

void
HTTP::_assignMime( void ) {
	File	file( nameMime, R );
	str_t	type, exts, ext;

	while ( !file.fs.eof() ) {
		file.fs >> type;
		
		std::getline( file.fs, exts, ';' );
		isstream_t	iss( exts );
		while ( iss >> ext )
			key.mime.insert( std::make_pair( ext, type ) );
	}
}

void
HTTP::_assignVec( vec_str_t& target, const str_t source[], size_t cnt ) {
	for ( size_t idx = 0; idx < cnt; ++idx )
		target.push_back( source[idx] );
}

/* METHOD - response: send response message */
void
HTTP::response( const Client& client, const Request& rqst ) {
	osstream_t oss;
	_message( Response( rqst ), oss );

	logfile.fs << oss.str() << std::endl;
	ssize_t bytesSent = send( client.socket(), oss.str().c_str(), oss.str().size(), 0 );

	if ( bytesSent == ERROR )
		throw err_t( "http: send: " + errMsg[FAIL_SEND] );
}

void
HTTP::_message( const Response& rspn, osstream_t& oss ) {
	_msgLine( rspn, oss );
	_msgHeader( rspn, oss );
	if ( rspn.body() )
		_msgBody( rspn, oss );
}

void
HTTP::_msgLine( const Response& rspn, osstream_t& oss ) {
	map_uint_str_t::iterator iter = key.status.find( rspn.line().status );

	oss <<
	http.signature << '/' << http.version.at( static_cast<size_t>( rspn.line().version ) ) << ' ' <<
	iter->first << " " << iter->second << 
	CRLF;
}

void
HTTP::_msgHeader( const Response& rspn, osstream_t& oss ) {
	for ( vec_uint_t::const_iterator iter = rspn.header().list.begin(); iter != rspn.header().list.end(); ++iter ) {
		_msgHeaderName( *iter, oss );
		_msgHeaderValue( rspn.header(), *iter, oss );
	}
	oss << CRLF;
}

void
HTTP::_msgHeaderName( uint_t id, osstream_t& oss ) {
	oss << key.header_out.at( id ) << ": ";
}

void
HTTP::_msgHeaderValue( const response_header_t& header, uint_t id, osstream_t& oss ) {
	switch( id ) {
		case OUT_SERVER: oss << header.server; break;
		case OUT_DATE: break;
		case OUT_CONNECTION: break;
		case OUT_CHUNK: break;
		case OUT_CONTENT_LEN: oss << header.content_length; break;
		case OUT_CONTENT_TYPE: oss << header.content_type; break;
	}
	oss << CRLF;
}

void
HTTP::_msgBody( const Response& rspn, osstream_t& oss ) {
	for ( size_t idx = 0; idx < rspn.header().content_length; ++idx )
		oss << rspn.body()[idx];
}
