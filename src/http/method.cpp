#include "HTTP.hpp"

char*
HTTP::GET( const str_t& uri, size_t& size ) {
	try {
		File target( config.dirRoot + uri, R_BINARY );

		std::filebuf* pbuf = target.fs.rdbuf();
		size = pbuf->pubseekoff( 0, target.fs.end, target.fs.in );
		pbuf->pubseekpos( 0, target.fs.in );

		char *buf = new char[size];
		pbuf->sgetn( buf, size );
		
		return buf;
	} catch ( exception_t& exc ) { return NULL; }
}
 
void
HTTP::POST( const Request& rqst ) {
	File target( config.dirRoot + rqst.line().uri, W );

	target.fs << rqst.body();
}

errno_t
HTTP::DELETE( const Request& rqst ) {
	stat_t	stat;

	if ( std::stat( rqst.line().uri, stat ) ) != ERROR )
		return std::remove( ( dirRoot + rqst.line().uri ).c_str() ) != ERROR;
}
