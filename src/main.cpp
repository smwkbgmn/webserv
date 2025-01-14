#include "Webserv.hpp"

int main(int argc, char* argv[]) {
	if (argc > 2) {
		std::cerr << "Only the first config file will be applied to the server\n";
	}

	try {
		Kqueue	event_interface;
		Webserv server(event_interface);

		server.init(argv[1]);
		server.run();
		
	} catch (exception_t &exc) {
		std::cerr << "webserv: " << exc.what() << '\n';

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
