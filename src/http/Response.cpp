
#include "Response.hpp"

Response::Response( const Request& rqst ): _body( NULL ) {
	// Check the configuration what method are allowed
	// When the method is known but not allowed, response
	// with status 405

	switch ( rqst.line().method ) {
		case GET:
			_body = HTTP::GET( rqst.line().uri, _header.content_length, rqst.config().root );
			if ( !_body ) {
				_body = HTTP::GET( rqst.config().file40x, _header.content_length, rqst.config().root );
				_line.status = 404;
			}
			_mime( rqst.line().uri, _header.content_type, HTTP::http.typeDefault );
			_header.list.push_back( OUT_CONTENT_TYPE );
			_header.list.push_back( OUT_CONTENT_LEN );
			break;

		case POST:
			// The POST can append data to or create target source
			// Do I have to send different status code?
			HTTP::POST( rqst, rqst.config().root );
			_line.status = 204;
			break;

		case DELETE:
			if ( HTTP::DELETE( rqst, rqst.config().root ) )
				_line.status = 204;
			else
				_line.status = 404;
			break;

		case NOT_ALLOWED:
			_pageError( 405, rqst.config() );
			break;

		default:
			_pageError( 400, rqst.config() );
	}
}

void
Response::_mime( const str_t& uri, str_t& typeHeader, const str_t& typeUnrecog ) {
	size_t pos = uri.rfind( '.' );
	
	if ( pos != str_t::npos ) {
		str_t ext = uri.substr( pos + 1 );

		try { typeHeader = HTTP::key.mime.at( ext ); }
		catch ( exception_t &exc ) { typeHeader = typeUnrecog; }
	}
}

Response::Response( const Client& client ): _body( NULL ) { 
	const config_t&	config = client.server().config().at( 0 );

	_pageError( 400, config );
	_mime( config.file40x, _header.content_type, HTTP::http.typeDefault );
}

void
Response::_pageError( const uint_t& status, const config_t& config ) {
	_line.status = status;	

	if ( status == 400 )
		_body = HTTP::GET( )
	if ( status < 500 )
		_body = HTTP::GET( config.file40x, _header.content_length );
	else
		_body = HTTP::GET( config.file50x, _header.content_length, config.root );

	_header.list.push_back( OUT_CONTENT_TYPE );
	_header.list.push_back( OUT_CONTENT_LEN );
}

Response::~Response( void ) { if ( _body ) delete _body; }

const response_line_t&
Response::line( void ) const { return _line; }

const response_header_t&
Response::header( void ) const { return _header; }

const char*
Response::body( void ) const { return _body; }


