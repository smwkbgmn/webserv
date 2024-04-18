#include "webserv.hpp"

/*
    CGI: autoindexing (GET/php script), RPN calculator (GET/execution), sort (POST/c++ script)
	- See if the query and environ vars are must be used

	To do
	- File upload
    - Handle chunked request/response
	- Redirect the error case to URI for error page
	- Apply corrected config structures
	- Add retrived location to Request obj after replace uri with real path
	- Implement cookies

	Improve
    - For efficiency, try replace the body type with stream 
	- Would it fit well if make header as map of enum header key and header value?
	it makes the working of header list and values as combined one

	Correction
	- filter.hpp
		allow type
	
*/

int main( void ) {
	try {
		HTTP::init( "text/plain", "html/cgi-bin" );
		
		Server server;

		// Retrieve http signature and default type from config file
		server.listening();
		// server.connect_sever();
	} catch ( err_t &err ) { clog( str_t( err.what() ) ); }

  return 0;
}