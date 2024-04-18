#include "HTTP.hpp"

http_t HTTP::http;
keys_t HTTP::key;

/* METHOD - init: assign basic HTTP info and load keys */
void
HTTP::init( const str_t& type, const name_t& cgi ) {
	http.signature		= "HTTP";
	http.typeDefault	= type;
	http.locationCGI	= cgi;
	http.fileAtidx		= cgi + "/autoindex_v2.cgi";

	_assignVec( http.version, strVersion, CNT_VERSION );
	_assignVec( http.method, strMethod, CNT_METHOD );

	_assignCWD();
	_assignHeader();
	_assignStatus();
	_assignMime();
}

void
HTTP::_assignHeader( void ) {
	str_t header;

	File fileIn( fileHeaderIn, R );
	while ( std::getline( fileIn.fs, header ) )
		key.header_in.push_back( header );

	File fileOut( fileHeaderOut, R );
	while ( std::getline( fileOut.fs, header ) ) key.header_out.push_back( header );
}

void
HTTP::_assignStatus( void ) {
	File file( fileStatus, R );

	while ( !file.fs.eof() ) {
		uint_t code;
		str_t reason;

		file.fs >> code;
		file.fs.get();
		std::getline( file.fs, reason );

		key.status.insert( std::make_pair( code, reason ) );
	}
}

void
HTTP::_assignMime(void) {
	File	file( fileMime, R );
	str_t	type, exts, ext;

	while ( !file.fs.eof() ) {
		file.fs >> type;

		std::getline( file.fs, exts, ';' );
		isstream_t iss( exts );
		while ( iss >> ext ) key.mime.insert( std::make_pair( ext, type ) );
	}
}

void
HTTP::_assignVec( vec_str_t& target, const str_t source[], size_t cnt ) {
	for ( size_t idx = 0; idx < cnt; ++idx )
		target.push_back(source[idx]);
}


void
HTTP::_assignCWD( void ) {
	// char buf[1024];
	// http.absolute.assign( getcwd( buf, 1024 ) );
	http.absolute = ".";
}


/* METHOD - getLocationConf: get index of vec_config_t matching with request URI
 */
size_t HTTP::getLocationConf( const str_t& uri, const vec_config_t& config ) {
	if ( config.size() > 1 ) {
		vec_config_t::const_iterator	iter	= config.begin();
		size_t							idx		= 1;

		while ( iter != config.end() ) {
			if ( uri.find( iter->location ) == 0)
				return idx;
			++iter;
			++idx;
		}
	}
	return 0;
}

/* FILTER INIT */
errstat_s::errstat_s( const uint_t& status ) { code = status; }

config_s::config_s( void ) {
	location		= "/";
	// root			= HTTP::http.absolute + "/html";
	root			= "html";
	file40x			= "/40x.html";
	file50x			= "/50x.html";

	atidx			= FALSE;
	sizeBodyMax		= 1000;

	allow.insert( std::make_pair( GET, TRUE ) );
	allow.insert( std::make_pair( POST, TRUE ) );
	allow.insert( std::make_pair( DELETE, TRUE ) );
}

request_header_s::request_header_s( void ) {
	connection		= KEEP_ALIVE;
	chunked			= FALSE;
	content_length	= 0;
}

response_line_s::response_line_s( void ) {
	version			= VERSION_11;
	status			= 200;
}

response_header_s::response_header_s( void ) {
	connection		= KEEP_ALIVE;
	chunked			= FALSE;
	content_length	= 0;
}

char* dupStreamBuffer( std::ios& obj, size_t& size ) {
	std::streambuf* pbuf = obj.rdbuf();
	size = pbuf->pubseekoff( 0, obj.end, obj.in );
	pbuf->pubseekpos( 0, obj.in );

	// std::clog << "dupStreamBuffer: before new char[" << size << "]\n";
	char* buf = new char[size];
	// std::clog << "dupStreamBuffer: before pbuf->sgetn\n";
	pbuf->sgetn( buf, size );

	return buf;
}
