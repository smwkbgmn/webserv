#include "Request.hpp"

/* ACCESS */
const Client&			Request::client( void ) const { return _client; }
const config_t&			Request::config( void ) const { return client().getServer().config().at( _configIdx ); }

const request_line_t&	Request::line( void ) const { return _line; }
const request_header_t&	Request::header( void ) const { return _header; }
const char*				Request::body( void ) const { return _body; }

/* CONSTRUCT */
Request::Request( const Client& client ): _client( client ), _body( NULL ) {
	log( "HTTP\t: constructing requeset" );

	const char* buf = client.buffer();

	_parse( buf );

	// If the method is not allowed at this location config, set method_e as NOT_ALLOWED
	if ( _line.method != UNKNOWN && !config().allow.at( _line.method ) )
		_line.method = NOT_ALLOWED;
}

void
Request::_parse( const char* buf ) {
	str_t	msgRqst( buf );
	size_t	begin	= 0;
	size_t	end		= 0;

	// CRLF could be replaced with only LF (see RFC)
	end = msgRqst.find( CRLF, begin );
	_parseLine( msgRqst.substr( begin, end ) );
	begin = end + 2;

	while ( ( end = msgRqst.find( CRLF, begin ) ) != str_t::npos ) {
		if ( end == begin ) {
			begin += 2;
			break;
		}
		
		_parseHeader( msgRqst.substr( begin, end ) );
		begin = end + 2;
	}

	if ( _header.content_length )
		_assignBody( begin, buf );
} 

void
Request::_parseLine( str_t line ) {
	isstream_t iss( line );

	_assignMethod( _token( iss, SP ) );
	_assignURI( _token( iss, SP ) );
	_assignVersion( _token( iss, NONE ) );
}

void
Request::_assignMethod( str_t token ) {
	vec_str_iter_t	iter = lookup( HTTP::http.method, token );

	if ( iter == HTTP::http.method.end() )
		_line.method = UNKNOWN;
	else
		_line.method = static_cast<method_e>( std::distance( HTTP::http.method.begin(), iter ) );
}

void
Request::_assignURI( str_t token ) { 
	_configIdx	= HTTP::getLocationConf( _line.uri, _client.getServer().config() );
	// _config		= _client.server().config().at( _configIdx );

	if ( config().location.length() == 1 )
		_line.uri = token.replace( 0, config().location.length(), config().root + "/" );
	else
		_line.uri = token.replace( 0, config().location.length(), config().root );

	size_t	pos_query = _line.uri.find( '?' );
	if ( pos_query != str_t::npos ) {
		_line.query = _line.uri.substr( pos_query + 1);
		_line.uri.erase( pos_query );
	}
}

void
Request::_assignVersion( str_t token ) {
	isstream_t iss( token );

	if ( _token( iss, '/' ) != HTTP::http.signature )
		throw err_t( "_assignVersion: " + errMsg[INVALID_REQUEST_LINE] );
	
	vec_str_iter_t iter = lookup( HTTP::http.version, _token( iss, NONE ) );
	if ( iter == HTTP::http.version.end() )
		_line.version = NOT_SUPPORTED;
	else
		_line.version = static_cast<version_e>( std::distance( HTTP::http.version.begin(), iter ) );
}

void
Request::_parseHeader( str_t field ) {
	isstream_t		iss( field );
	str_t			header;

	header = _token( iss, ':' );
	iss >> std::ws;

	switch ( _add( _header.list, distance( HTTP::key.header_in, header ) ) ) {
		case IN_HOST		: iss >> _header.host; break;
		case IN_CONNECTION	: _header.connection = KEEP_ALIVE; break;
		case IN_CHUNK		: break;
		case IN_CONTENT_LEN	: iss >> _header.content_length; break;
		case IN_CONTENT_TYPE: iss >> _header.content_type; break;
	}
}

ssize_t
Request::_add( vec_uint_t& list, ssize_t id ) { if ( id != -1 ) list.push_back( id ); return id; }

void
Request::_assignBody( const size_t& bodyBegin, const char* buf ) {
	_body = new char[_header.content_length];
	memcpy( _body, &buf[bodyBegin], _header.content_length );
	
	// log( "HTTP\t: the rqst body" );
	// std::clog.write( _body, _header.content_length );
	// std::clog << std::endl;
}

str_t
Request::_token( isstream_t& iss, char delim ) {
	str_t token;

	if ( ( delim && !std::getline( iss, token, delim ) ) ||
		( !delim && !std::getline( iss, token ) ) )
		throw err_t( "_token: " + errMsg[INVALID_REQUEST_LINE] );

	return token;
}

Request::Request( const Client& client, const config_t& config, const uint_t& status ): _client( client ), _body( NULL ) {
	_line.version	= VERSION_11;
	_line.method	= GET;

	if ( status < 500 ) {
		_line.uri = config.file40x;
		
		// size_t pos_dot = _line.uri.rfind( '.' );
		// if ( pos_dot != str_t::npos && _line.uri.substr( pos_dot ) == ".cgi" ) {
		// 	osstream_t query;
		// 	query << "status_code=" << status << "&explanation=";
			
		// 	str_t	explain( HTTP::key.status.at( status ) );
		// 	size_t	pos_ws;

		// 	while ( ( pos_ws = explain.find( ' ' ) ) != str_t::npos )
		// 		explain.replace( pos_ws, 1, "+" );
		// 	query << explain;

		// 	_line.query = query.str();
		// }
	}
	else _line.uri = config.file50x;

	// _config = config;
}

Request::~Request( void ) { if ( _body ) delete _body; }

