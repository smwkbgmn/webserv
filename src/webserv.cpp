#include "webserv.hpp"

/*
	- vary responses when fail to do method or incorrect message
	- CGI
	- redirect
*/

int main( void ) {	
	try {
		Server	server;
		
		HTTP::init();
		server.listening();
	} catch ( err_t &err ) { std::cerr << err.what() << std::endl; }

	return 0;
}