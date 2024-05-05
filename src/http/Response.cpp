
#include "Response.hpp"

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
			} catch ( errstat_t& errstat ) { _redirect( errstat.code, rqst.config() ); }
			break;

		case POST:
			// The POST can append data to or create target source
			// Do I have to send different status code?
			try {
				HTTP::POST( rqst, &_body, _header.content_length );
				_line.status = 204;
			} catch ( errstat_t& errstat ) { _redirect( errstat.code, rqst.config() ); }
			break;

		case DELETE:
			try {
				HTTP::DELETE( rqst );
				_line.status = 204;
			} catch ( errstat_t& errstat ) { _redirect( errstat.code, rqst.config() ); }
 			break;

		case NOT_ALLOWED:
			_redirect( 405, rqst.config() );
			break;
			  

		default:
			_redirect( 400, rqst.config() );
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

	_redirect( errstat, config );
	// _errpage( client, errstat );
}

// void
// Response::_errpage( const Client& client, const config_t& config, const uint_t& status ) {
// 	timestamp();
// 	std::clog << "HTTP\t: responsing with errcode " << status << "\n";

// 	_line.status = status;

// 	
// 		if ( config.file40x.empty() ) _errpageBuild( status );
// 		else HTTP::GET( Request( client, config, status ), &_body, _header.content_length );
// 	}
// 	if ( ( status < 500 && config.file40x.empty() ) ||
// 		config.file50x.empty() ) _errpageBuild( status );
// 	else ;
// }

// void
// Response::_errpageBuild( const uint_t& status ) {
// 	osstream_t	errpage;

// 	_body					= dupStreamBuf( errpage, _header.content_length );
// 	_header.content_type	= "text/html";
// }

void
Response::_redirect( const uint_t& status, const config_t& config ) {
	timestamp();
	std::clog << "HTTP\t: responsing with errcode " << status << "\n";

	_line.status = 307;	

	osstream_t query;
	query << "?status_code=" << status << "&explanation=";
	
	str_t	explain( HTTP::key.status.at( status ) );
	size_t	pos_ws;

	while ( ( pos_ws = explain.find( ' ' ) ) != str_t::npos )
		explain.replace( pos_ws, 1, "+" );
	query << explain;

	if ( status < 500 )
		_header.location = "http://localhost:8080" + config.file40x + query.str();
	else
		_header.location = "http://localhost:8080" + config.file50x;

	_header.list.push_back( OUT_LOCATION );
}

Response::~Response( void ) { if ( _body ) delete _body; }

const response_line_t&
Response::line( void ) const { return _line; }

const response_header_t&
Response::header( void ) const { return _header; }

const char*
Response::body( void ) const { return _body; }


