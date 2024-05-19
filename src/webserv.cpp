#include "webserv.hpp"

/*
	To do
	- CGI 
		Define a directory or a file from where the file should be searched
		(for example, if url /kapouet is rooted to /tmp/www,
		url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet)

		Execute CGI based on certain file extension (for example .php)

		Make the route able to accept uploaded files and configure where they should be saved

		Just remember that, for chunked request, your server needs to unchunk it, the CGI will expect EOF as end of the body
		Same things for the output of the CGI.
		If no content_length is returned from the CGI, EOF will mark the end of the returned data

	- Chunked request
	- Config - server_name & location

	- Check NGINX how the config rule be applied to following dir or file
	- Add some checkign for invalid request at Request
	- Rewrite redirection
	- Implement cookies
	> RFC 6265

	Considertion
	- Would it fit well making header as map of enum header key and header value?
	it change the working of header list and values as combined one
	- Take some time to think of what would happen when the request msg is splited
	by buffer size and it cause the fractured part at the end of request taht is
	resulting unavailable to find the "\r\n\r\n" while taking request
	- Add program option for toggle of logging
	- See if other connection headers should be handled 
	- When send response, intead of copying the body at the building message, 
	just build the msg and use body directly from the response object
*/

int main( void ) {
	try {
		HTTP::init();
		CGI::init();
		
		Server server;

		server.connectsever();
	} catch ( err_t &err ) { log( str_t( err.what() ) ); return EXIT_FAILURE; }

  return 0;
}