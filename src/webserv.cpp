#include "webserv.hpp"

/*
	To do
	- Chunked request
	- Add Date header at addServerInfo
	- Implement cookies
		> RFC 6265

	- Add some checkign for invalid request at Request

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
	- Could be message buffer in the Trasaction object?
*/

int main( void ) {
	try {
		vec_config_t confs;
		confs.push_back( config_t() );
		// parseConfig( confs, argv[1] );

		HTTP::init();

		Server server( confs );
		server.connectsever();

		return EXIT_SUCCESS;
	}
	catch ( err_t &err ) { std::clog << "check 3-1\n"; log( str_t( err.what() ) ); return EXIT_FAILURE; }
	
	return EXIT_FAILURE;
}

// int main( int argc, char* argv[] ) {
// 	if ( argc == 1 ) {
// 		try {
// 			vec_config_t confs;
// 			parseConfig( confs, argv[1] );

// 			HTTP::init();
// 			CGI::init();

// 			Server server( confs );
// 			server.connectsever();

// 			return EXIT_SUCCESS;
// 		}
// 		catch ( err_t &err ) { log( str_t( err.what() ) ); return EXIT_FAILURE; }
// 	}
// 	return EXIT_FAILURE;
// }