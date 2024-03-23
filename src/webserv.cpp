#include "webserv.hpp"

int main( void ) {	
	try {
		Server	server;
		
		server.listening();
	} catch ( err_t &err ) { std::cerr << err.what() << std::endl; }

	return 0;
}