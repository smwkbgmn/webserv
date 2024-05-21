#include "webserv.hpp"

// int main( void ) {
// 	try {
// 		vec_config_t confs;
// 		confs.push_back( config_t() );

// 		HTTP::init();

// 		Server server( confs );
// 		server.connectsever();

// 		return EXIT_SUCCESS;
// 	}
// 	catch ( err_t &err ) { std::clog << "check 3-1\n"; log( str_t( err.what() ) ); return EXIT_FAILURE; }
	
// 	return EXIT_FAILURE;
// }

int main( int argc, char* argv[] ) {
	if ( argc == 2 ) {
		try {
			vec_config_t confs;
			parseConfig( confs, argv[1] );

			HTTP::init();
			CGI::init();

			Server server( confs );
			server.connectsever();

			return EXIT_SUCCESS;
		}
		catch ( err_t &err ) { log( str_t( err.what() ) ); return EXIT_FAILURE; }
	}
	return EXIT_FAILURE;
}