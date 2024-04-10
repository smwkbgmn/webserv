#include "webserv.hpp"

/*
	- Vary responses when fail to do method or incorrect message
	- CGI > file upload, calculator
	- Redirecttion
*/

int main( void ) {	
	try {
		Server	server;
		
		// Retrieve http signature and default type from config file
		HTTP::init( "HTTP", "text/plain" );
		server.listening();
	} catch ( err_t &err ) { std::cerr << err.what() << std::endl; }

	return 0;
}