
#include "Response.hpp"

const response_line_t&		Response::line( void ) const { return _line; }
const response_header_t&	Response::header( void ) const { return _header; }
const char*					Response::body( void ) const { return _body; }

Response::Response( const Request& rqst ): _body( NULL ) {
	/*
		The CGI could be invoked by several tokens
		1. extension	: POST, .exe
		2. location		: GET, /cgi-bin
		3. autoindex	: GET, /
	*/ 

	// Check the configuration what method are allowed
	// When the method is known but not allowed, response
	// with status 405

	switch ( rqst.line().method ) {
		case GET:
			try {
				if ( rqst.body() || rqst.header().content_length || !rqst.header().content_type.empty() )
					throw errstat_t( 400, ": the GET request may not be with body" );

				HTTP::GET( rqst, &_body, _header.content_length );
				_mime( rqst.line().uri, _header.content_type, HTTP::http.typeDefault );
				_header.list.push_back( OUT_CONTENT_LEN );
				_header.list.push_back( OUT_CONTENT_TYPE );
			} catch ( errstat_t& errstat ) { _errpage( errstat.code, rqst.config() ); }
			break;

		case POST:
			// The POST can append data to or create target source
			// Do I have to send different status code?
			try {
				HTTP::POST( rqst, &_body, _header.content_length );
				_line.status = 204;
			} catch ( errstat_t& errstat ) { _errpage( errstat.code, rqst.config() ); }
			break;

		case DELETE:
			try {
				HTTP::DELETE( rqst );
				_line.status = 204;
			} catch ( errstat_t& errstat ) { _errpage( errstat.code, rqst.config() ); }
 			break;

		case NOT_ALLOWED:
			_errpage( 405, rqst.config() );
			break;
		
		case UNKNOWN:
			map_method_bool_t::const_iterator iter = rqst.config().allow.begin();
			while ( iter != rqst.config().allow.end() )
				if ( iter->second ) _header.allow.push_back( iter->first );
			_header.list.push_back( OUT_ALLOW );
			_errpage( 405, rqst.config() );
			
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

Response::Response( const Client& client, const uint_t& errstat ): _body( NULL ) { 
	const config_t&	config = client.getServer().config().at( 0 );

	_errpage( errstat, config );
}

void
Response::_errpage( const uint_t& status, const config_t& config ) {
	timestamp();
	std::clog << "HTTP\t: responsing with errcode " << status << "\n";

	path_t	page;

	if ( status < 500 ) page = config.file40x;
	else page = config.file50x;

	if ( !page.empty() && isExist( page ) ) HTTP::GET( page, &_body, _header.content_length );
	else _errpageBuild( status );

	_header.content_type = HTTP::key.mime.at( "html" );
	_header.list.push_back( OUT_CONTENT_LEN );
	_header.list.push_back( OUT_CONTENT_TYPE );
	_line.status = status;
}

void
Response::_errpageBuild( const uint_t& status ) {
	sstream_t page;

	errpageScript( page, status, HTTP::key.status.at( status ) );
	_body = dupStreamBuf( page, _header.content_length );
}

Response::~Response( void ) { if ( _body ) delete _body; }
