#include "Request.hpp"

Request::Request( const Client& client ): _client( client ), _body( NULL ) {
	// timestamp();
	// std::clog << "constructing rqst - " << client.socket() << std::endl;
	// std::clog << client.buffer() << std::endl;

	// Parse request message
	_parse( client.buffer() );

	// Set config based by location
	_configIdx = HTTP::getLocationConf( _line.uri, client.server().config() );

	// If the method is not allowed at this location config, set methodID as NOT_ALLOWED
	if ( _line.method != UNKNOWN && !config().allow.at( _line.method ) )
		_line.method = NOT_ALLOWED;
	// clog("constructing rqst");
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
	
	// std::clog << "pos begin: " << begin << ", msg len: " << msgRqst.length() << std::endl;
	if ( begin != msgRqst.length() ) {
		// _parseBody( msgRqst.substr( begin ) );
		size_t bodysize = client().byte_read - begin;
		_body = new char[bodysize];
		memcpy( _body, &buf[begin], bodysize );
		// std::clog << "the size of body: " << bodysize << "\n";
		// for ( size_t idx = 0; idx < bodysize; ++idx )
		// 	std::clog << _body[idx];
		// std::clog << "body end" << std::endl;
	}

	// LOGGING Request Message
	logging.fs << msgRqst << std::endl;
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
		_line.method = static_cast<methodID>( std::distance( HTTP::http.method.begin(), iter ) );
}

void
Request::_assignURI( str_t token ) { _line.uri = token; }

void
Request::_assignVersion( str_t token ) {
	isstream_t iss( token );

	if ( _token( iss, '/' ) != HTTP::http.signature )
		throw err_t( "_assignVersion: " + errMsg[INVALID_REQUEST_LINE] );
	
	vec_str_iter_t iter = lookup( HTTP::http.version, _token( iss, NONE ) );
	if ( iter == HTTP::http.version.end() )
		_line.version = NOT_SUPPORTED;
	else
		_line.version = static_cast<versionID>( std::distance( HTTP::http.version.begin(), iter ) );
}

void
Request::_parseHeader( str_t field ) {
	isstream_t		iss( field );
	str_t			name;

	// if ( iter == HTTP::header_in.end() )
	// 	throw err_t( "_parseHeader: " + errMsg[INVALID_REQUEST_FIELD] + " " + field );

	// std::clog << "rqst - header: " << field << std::endl;

	
	name = _token( iss, ':' );
	iss >> std::ws;

	vec_str_iter_t	iter = lookup( HTTP::key.header_in, name );

	switch ( std::distance( HTTP::key.header_in.begin(), iter ) ) {
		case IN_HOST		: iss >> _header.host; _add( _header.list, IN_HOST ); break;
		case IN_CONNECTION	: _header.connection = KEEP_ALIVE; _add( _header.list, IN_CONNECTION ); break;
		case IN_CHUNK		: break;
		case IN_CONTENT_LEN	: iss >> _header.content_length; _add( _header.list, IN_CONTENT_LEN );
		case IN_CONTENT_TYPE: break;
		// default: throw err_t( "_parseHeader: " + errMsg[INVALID_REQUEST_FIELD] + " " + field );
	}
}

void
Request::_add( vec_uint_t& list, uint_t id ) { list.push_back( id ); }

void
Request::_parseBody( str_t body ) {
	( void )body;
	// clog( "HTTP - _parseBody" );
	// std::clog << "tokened the body\n";
	// std::clog << body << std::endl;

	// _body = new char[body.length()];
	// body.copy( _body, body.length() );
	// _body[body.length()] = '\0';
}

str_t
Request::_token( isstream_t& iss, char delim ) {
	str_t token;

	if ( ( delim && !std::getline( iss, token, delim ) ) ||
		( !delim && !std::getline( iss, token ) ) )
		throw err_t( "_token: " + errMsg[INVALID_REQUEST_LINE] );

	return token;
}

Request::~Request( void ) { if ( _body ) delete _body; }

const Client&
Request::client( void ) const { return _client; }

const config_t& 
Request::config( void ) const { return client().server().config().at( _configIdx ); }

const request_line_t&
Request::line( void ) const { return _line; }

const request_header_t&
Request::header( void ) const { return _header; }

const char*
Request::body( void ) const { return _body; }




