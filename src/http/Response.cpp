
#include "Response.hpp"

const response_line_t&		Response::line( void ) const { return _line; }
const response_header_t&	Response::header( void ) const { return _header; }
const sstream_t&			Response::body( void ) const { return _body; }

Response::Response( const Request& rqst ) {
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
			if ( isDir( rqst.info ) ) _index( rqst );
			else {
				try {
					if ( rqst.body().str().length() || rqst.header().content_length || !rqst.header().content_type.empty() )
						throw errstat_t( 400, ": the GET request may not be with body" );

					HTTP::GET( rqst.line().uri, _body, _header.content_length );
					_mime( rqst.line().uri );
					_header.list.push_back( OUT_CONTENT_LEN );
					_header.list.push_back( OUT_CONTENT_TYPE );
				}
				catch ( errstat_t& errstat ) { _errpage( errstat.code, rqst.config() ); }
			}
			break;

		case POST:
			// The POST can append data to or create target source
			// Do I have to send different status code?
			try {
				HTTP::POST( rqst, _body, _header.content_length );
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
			vec_uint_t::const_iterator iter = rqst.location().allow.begin();
			while ( iter != rqst.location().allow.end() )
				_header.allow.push_back( *iter );
			_header.list.push_back( OUT_ALLOW );
			_errpage( 501, rqst.config() );
			break;
	}
}

void
Response::_mime( const str_t& uri ) {
	size_t pos = uri.rfind( '.' );
	
	if ( pos != str_t::npos ) {
		str_t ext = uri.substr( pos + 1 );

		try { _header.content_type = HTTP::key.mime.at( ext ); }
		catch ( exception_t &exc ) { _header.content_type = HTTP::http.type_unknown; }
	}
}

Response::Response( const Client& client, const uint_t& errstat ) { 
	_errpage( errstat, client.server().config() );
}

void
Response::_index( const Request& rqst ) {
	if ( *rqst.line().uri.rbegin() != '/' ) _redirect( _indexURIConceal( rqst, "" ) + "/", 301 );
	else {
		path_t	index;
		fstat_t	index_info;

		if ( rqst.location().index.size() && !( index = _indexValid( rqst, index_info ) ).empty() ) {
			if ( isDir( index_info ) ) _redirect( _indexURIConceal( rqst, index ) + "/", 301 );
			else HTTP::GET( index, _body, _header.content_length );
		}
		else {
			if ( rqst.location().index_auto ) {
				autoindexScript( rqst.line().uri, _body );
				_header.content_type	= HTTP::key.mime.at( "html" );
				_header.content_length	= _body.str().size();
			}
			else _errpage( 403, rqst.config() );
		}
	}
}

path_t
Response::_indexValid( const Request& rqst, fstat_t& info ) {
	for ( vec_str_t::const_iterator iter = rqst.location().index.begin();
		iter != rqst.location().index.end(); ++iter ) {
		if ( getInfo( rqst.line().uri + *iter, info ) ) 
			return *iter;
	}
	return "";
}

path_t
Response::_indexURIConceal( const Request& rqst, const path_t& index  ) {
	path_t	concealed = rqst.location().alias + rqst.line().uri.substr( rqst.location().root.length() );

	if ( !index.empty() )
		concealed += "/" + index;

	std::clog << "_uriConceal: " << concealed << std::endl;

	return concealed;
}

void
Response::_redirect( const path_t& dest, const uint_t& status ) {
	_line.status		= status;
	_header.location	= dest;
	_header.list.push_back( OUT_LOCATION );
}

void
Response::_errpage( const uint_t& status, const config_t& config ) {
	timestamp();
	std::clog << "HTTP\t: responsing with errcode " << status << "\n";

	path_t	page;

	if ( status < 500 ) page = config.file_40x;
	else page = config.file_50x;

	if ( !page.empty() && isExist( page ) ) HTTP::GET( page, _body, _header.content_length );
	else _errpageBuild( status );

	_header.content_type = HTTP::key.mime.at( "html" );
	_header.list.push_back( OUT_CONTENT_LEN );
	_header.list.push_back( OUT_CONTENT_TYPE );
	_line.status = status;
}

void
Response::_errpageBuild( const uint_t& status ) {
	errpageScript( _body, status, HTTP::key.status.at( status ) );
	_header.content_length = _body.str().size();
}

Response::~Response( void ) {}

/* STRUCT */
response_line_s::response_line_s( void ) {
	version			= VERSION_11;
	status			= 200;
}

response_header_s::response_header_s( void ) {
	connection		= KEEP_ALIVE;
	chunked			= FALSE;
	content_length	= 0;
}
