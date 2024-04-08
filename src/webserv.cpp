#include "webserv.hpp"

/*
	- method DELETE
	- MIME init
	- header_out structure
	- error handled response
*/

int main( void ) {	
	try {
		Server	server;
		
		HTTP::init();
		server.listening();
	} catch ( err_t &err ) { std::cerr << err.what() << std::endl; }

	return 0;
}