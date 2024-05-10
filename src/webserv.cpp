#include "webserv.hpp"

/*
	To do
	- CGI
		Replace _wait NONE mode with WNOHANG
	- Make GET method to check all index files in case of target not found 
    - Handle chunked request/response
	- Rewrite redirection
	- Implement cookies

	Done
	- CGI
		Add building of argument and envs for CGI
		-> retrieve the PATH_INFO and QUERY_STRING from the URI
		CGI header build
	- Replace buffers
	- Apply corrected config structures
	- Add seeing how the file stat() is before proceeding HTTP methods
	- File upload
	- Add retrived location to Request obj after replace uri with real path
	- Redirect to error page in case of error in URI 

	Considertion
    - For efficiency, try to replace the body type with stream 
	- Would it fit well making header as map of enum header key and header value?
	it change the working of header list and values as combined one
	- Try default value by declaring directly -> path_t	location = "/";
	- Split buffer as in two of Request Line + Header Field and Body part
	- Take some time to think of what would happen when the request msg is splited
	by buffer size and it cause the fractured part at the end of request taht is
	resulting unavailable to find the "\r\n\r\n" while taking request
	- Add program option for toggle of logging
	- See if other connection headers should be handled 
*/

int main( void ) {
	try {
		HTTP::init();
		
		Server server;

		server.connectsever();
	} catch ( err_t &err ) { log( str_t( err.what() ) ); }

  return 0;
}