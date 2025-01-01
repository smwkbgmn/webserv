#include "Webserv.hpp"

int main(int argc, char* argv[]) {
	if (argc > 2) {
		log::print("Only the first config file will be applied to server");
	}

	try {
		Kqueue	event_interface;
		Webserv server(event_interface);

		server.init(argv[1]);
		server.run();
	} catch (exception_t &exc) { log::print(str_t(exc.what())); }

	return EXIT_SUCCESS;
}
