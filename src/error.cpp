#include "error.hpp"

void
throwSysErr( const str_t& fncName, uint_t code ) {
	perror( fncName.c_str() );
	logging.fs << "errno: " << errno;
	throw errstat_t( code );
}