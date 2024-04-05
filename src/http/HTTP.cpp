#include "HTTP.hpp"

/*
	1. Client send request for connection by browser 
	2. Accept connection through socket(UNIX)
	3. Receve request
	4. Send response
*/

str_t		HTTP::http;
vec_str_t	HTTP::version;
vec_str_t	HTTP::method;
status_t	HTTP::status;
mime_t		HTTP::mime;

void
HTTP::init( const str_t& nameStatus, const str_t& nameMime ) {
	http = "HTTP";
	_assignVec( version, strVersion, CNT_VERSION );
	_assignVec( method, strMethod, CNT_METHOD );
	
	File fileStatus( nameStatus, R );
	while ( )
}

void
HTTP::_assignVec( vec_str_t& target, const str_t source[], size_t cnt ) {
	for ( size_t idx = 0; idx < cnt; ++idx )
		target.push_back( source[idx] );
}
