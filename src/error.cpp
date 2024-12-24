#include "error.hpp"

void
throwSysErr( const str_t& fname ) {
	sstream_t ss;

	ss << fname + ": " + std::to_string(errno);
	perror( ss.str().c_str() );
	throw errstat_t( 500, 0 );
}

void
throwSysErr( const str_t& fname, const uint_t& code ) {
	sstream_t ss;
	
	ss << fname + ": " + std::to_string(errno);
	perror( ss.str().c_str() );
	throw errstat_t( code, 0 );
}

void
throwSysErr( const str_t& fname, const uint_t& code, const size_t& confidx ) {
	sstream_t ss;
	
	ss << fname + ": " + std::to_string(errno);
	perror( ss.str().c_str() );
	throw errstat_t( code, confidx );
}

errstat_s::errstat_s( const uint_t& status ): err_t( "system function failure" ), code( status ), confidx( 0 ) {}
errstat_s::errstat_s( const uint_t& status, const size_t& idx ): err_t( "system function failure" ), code( status ), confidx( idx ) {}
errstat_s::errstat_s( const uint_t& status, const str_t& msg ): err_t( msg ), code( status ), confidx( 0 ) {}
errstat_s::errstat_s( const uint_t& status, const str_t& msg, const size_t& idx ): err_t( msg ), code( status ), confidx( idx ) {}