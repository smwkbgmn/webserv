#include "webserv.hpp"

/*
    CGI: autoindexing (GET/php script), RPN calculator (GET/execution), sort (POST/c++ script)
	- See if the query and environ vars are must be used

	To do
	- File upload
	- Add building of argument and envs for CGI
	- Make GET method to check all index files in case of target not found 
	- Add seeing how the file stat is before proceeding HTTP methods
    - Handle chunked request/response
	- Apply corrected config structures
	- Add program option for toggle of logging
	- Implement cookies

	Done
	- Add retrived location to Request obj after replace uri with real path
	- Redirect to error page in case of error in URI 

	Improve
    - For efficiency, try replace the body type with stream 
	- Would it fit well if make header as map of enum header key and header value?
	it makes the working of header list and values as combined one
	- Try default value by declaring directly > path_t	location = "/";
*/

int main( void ) {
	try {
		HTTP::init( "text/plain", "html/cgi-bin" );
		
		Server server;

		// server.listening();
		server.connectsever();
	} catch ( err_t &err ) { clog( str_t( err.what() ) ); }

  return 0;
}