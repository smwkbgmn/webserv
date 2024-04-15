#include "webserv.hpp"

/*
    CGI: autoindexing (GET/php script), RPN calculator (GET/execution), sort (POST/c++ script)
	- See if the query and environ vars are must be used

	To do
    - Handle chunked request/response
	- Redirect the error case to URI for error page
	- Implement cookies
	- Apply corrected config structures

	Improve
    - For efficiency, try replace the body type with stream 
	- Would it fit well if make header as map of enum header key and header value?
	it makes the working of header list and values as combined one

*/

int main(void) {
	try {
		Server server;

		// Retrieve http signature and default type from config file
		HTTP::init("text/plain", "/cgi-bin");
		// server.listening();
		server.connect_sever();
	} catch (err_t &err) { std::cerr << err.what() << std::endl; }

  return 0;
}