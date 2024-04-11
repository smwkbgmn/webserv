#include "CGI.hpp"

bool
CGI::GET( const Request& rqst, char** bufptr, size_t size ) {
	
	// get autoindex
	if ( *rqst.line().uri.rbegin() == '/' )
		

}

bool
CGI::POST( const Request& rqst, char** bufptr, size_t size ) {
	
}

pid_t
CGI::_detach( pipe_t fd[2] ) {
}
