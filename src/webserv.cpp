#include "webserv.hpp"

int
main( int argc, char* argv[] ) {
	if ( argc == 2 ) {
		try {
			vec_config_t confs;
			parseConfig( confs, argv[1] );

			HTTP::init();

			Server server( confs);
			server.connectsever(confs);
		}
		catch ( err_t &err ) { log( str_t( err.what() ) ); return EXIT_FAILURE; }
	}
	return EXIT_FAILURE;
}
