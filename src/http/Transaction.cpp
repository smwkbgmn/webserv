#include "Transaction.hpp"

/*
	HTTP MESSAGE FORMMAT

	HTTP-message   = start-line CRLF
                   *( field-line CRLF )
                   CRLF
                   [ message-body ]

	request-line = method SP request-target SP HTTP-version
	status-line = HTTP-version SP status-code SP [ reason-phrase ]
*/

Transaction::Transaction( const str_t& msgRequest ): Request( msgRequest ) {}



/* BASE - REQUEST */
Request::Request( const str_t& msgRequest ) {
	size_t pos = 0;

	// CRLF could be replaced with only LF (see RFC)
	_getLine( msgRequest.substr( pos, pos = msgRequest.find( CRLF, pos ) ) );
	_getHeader( msgRequest.substr( ++pos, pos = msgRequest.find( CRLF, pos ) ) );
	// _getBody(  );
}

void
Request::_getLine( str_t line ) {
	isstream_t iss( line );

	_assignMethod( _token( iss, SP ) );
	_assignURI( _token( iss, SP ) );
	_assignVersion( _token( iss, NONE ) );
}

void
Request::_assignMethod( str_t token ) {
	int	id = 0;
	while ( id < CNT_METHOD && token != methodName[id] )
		id++;

	if ( id == CNT_METHOD )
		throw err_t( "_assignMethod: " + errMsg[INVALID_REQUEST_LINE] );
	
	_line.method = static_cast<methodID>( id );
}

void
Request::_assignURI( str_t token ) { _line.target = token; }

void
Request::_assignVersion( str_t token ) {
	isstream_t iss( token );

	if ( _token( iss, '/' ) != httpName )
		throw err_t( "_assignVersion: " + errMsg[INVALID_REQUEST_LINE] );
	
	vec_str_iter_t iter =
		std::find( httpVersion.begin(), httpVersion.end(), _token( iss, NONE ) );

	if ( iter == httpVersion.end() )
		throw err_t( "_assignVersion: " + errMsg[INVALID_REQUEST_LINE] );

	_line.version = std::distance( httpVersion.begin(), iter );
}




void
Request::_getHeader( str_t token ) {
	
}





str_t
Request::_token( isstream_t& iss, char delim ) {
	str_t token;

	if ( ( delim && !std::getline( iss, token, delim ) ) ||
		( !delim && !std::getline( iss, token ) ) )
		throw err_t( "_token: " + errMsg[INVALID_REQUEST_LINE] );

	return token;
}

/* BASE - RESPONSE */



