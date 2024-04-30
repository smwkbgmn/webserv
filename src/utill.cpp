#include "utill.hpp"

char*
dupStreamBuf( std::ios& obj, size_t& size ) {
	std::streambuf* pbuf = obj.rdbuf();
	size = pbuf->pubseekoff( 0, obj.end, obj.in );
	pbuf->pubseekpos( 0, obj.in );

	char* buf = new char[size];
	pbuf->sgetn( buf, size );

	return buf;
}

const char*
dupStreamBuf( const std::ios& obj, size_t& size ) {
	std::streambuf* pbuf = obj.rdbuf();
	size = pbuf->pubseekoff( 0, obj.end, obj.in );
	pbuf->pubseekpos( 0, obj.in );

	char* buf = new char[size];
	pbuf->sgetn( buf, size );

	return buf;
}

// char*
// dupStreamBuf( const osstream_t& obj ) {
//     std::streambuf* pbuf = obj.rdbuf();

// 	std::string	data;

// 	char buf[100];
// 	std::streamsize	read;
// 	while ( ( read = pbuf->sgetn( buf, 100 ) ) > 0 )
// 		data.append( buf, read );

// 	char* newbuf = new char[data.size()];
// 	memcpy( newbuf, data.data(), data.size() );
// 	return newbuf;
// }


