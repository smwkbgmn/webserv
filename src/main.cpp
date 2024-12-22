#include "Webserv.hpp"

int main(int argc, char* argv[]) {
	if (argc > 2) {
		log::print("Only a config file can be accepted.");
		return EXIT_FAILURE;
	}

	try {
		Kqueue	event_interface;
		Webserv core(event_interface);

		core.init(argv[1]);
		core.run();
	}
	catch (exception_t &exc) { log::print(str_t(exc.what())); }

	return EXIT_SUCCESS;
}
