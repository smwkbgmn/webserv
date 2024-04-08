#include "HTTP.hpp"

char*
HTTP::GET( const str_t& uri, str_t& type, size_t& size ) {
	File target( dirRoot + uri, R_BINARY );

	_extension( uri, type );
	std::filebuf* pbuf = target.fs.rdbuf();
	size = pbuf->pubseekoff( 0, target.fs.end, target.fs.in );
	pbuf->pubseekpos( 0, target.fs.in );

	char *buf = new char[size];
	target.fs.read( buf, size );
	// pbuf->sgetn( buf, size );
	
	return buf;
}
 
void
HTTP::_extension( const str_t& uri, str_t& type ) {
	size_t pos = uri.rfind( '.' );
	
	if ( pos != str_t::npos ) {
		str_t ext = uri.substr( pos + 1 );
		try { type = HTTP::mime.at( ext ); }
		catch ( exception_t &exc ) {}
	}
}

void
HTTP::POST( const Request& rqst ) {
	File target( dirRoot + rqst.line().uri, W );

	target.fs << rqst.body();
}

// void
// HTTP::DELETE( const Request& rqst ) {
	
// }
