#include "Webserv.hpp"

int main(char* argv[]) {
	try {
		Kqueue	event_interface;
		Webserv server(event_interface);

		server.init(argv[1]);
		server.run();
	} catch (exception_t &exc) { log::print(str_t(exc.what())); }

	return EXIT_SUCCESS;
}
