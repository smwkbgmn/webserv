#include "HTTP.hpp"

http_t HTTP::http;
keys_t HTTP::key;

/* METHOD - init: assign basic HTTP info and load keys */
void
HTTP::init( void ) {
	http.signature			= "HTTP";
	http.file_autoindex		= dir_cgi + "autoindex_v5.cgi";
	http.type_unknown		= "text/plain";

	_assignVec( http.version, str_version, CNT_VERSION );
	_assignVec( http.method, str_method, CNT_METHOD );

	_assignHeader();
	_assignStatus();
	_assignMime();

	CGI::init();
}

void
HTTP::_assignHeader( void ) {
	str_t header;

	File fileIn( file_header_in, READ );
	while ( std::getline( fileIn.fs, header ) )
		key.header_in.push_back( header );

	File fileOut( file_header_out, READ );
	while ( std::getline( fileOut.fs, header ) ) key.header_out.push_back( header );
}

void
HTTP::_assignStatus( void ) {
	File file( file_status, READ );

	while ( !file.fs.eof() ) {
		uint_t	code;
		str_t	reason;

		file.fs >> code;
		file.fs.get();
		std::getline( file.fs, reason );

		key.status.insert( std::make_pair( code, reason ) );
	}
}

void
HTTP::_assignMime(void) {
	File	file( file_mime, READ );
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

/* METHOD - getLocationConf: get index of vec_config_t matching with request URI */
size_t HTTP::setLocation( const str_t& uri, const vec_location_t& locations ) {
	if ( locations.size() > 1 ) {
		vec_location_t::const_iterator	iter	= locations.begin();
		size_t							idx		= 1;

		while ( iter != locations.end() ) {
			if ( uri.find( iter->alias ) == 0)
				return idx;
			++iter;
			++idx;
		}
	}
	return 0;
}

/* STURCT */
config_s::config_s( void ) {
	name			= "webserv";
	listen			= 8080; // mandatory
	
	root			= "html/"; // mandatory

	client_max_body	= 10240;

	/*
		The root configuration (i.e. location for "/") could be set or not and
		either case the location conf for the root MUST exist. It works as the 
		default.
	*/
	locations.push_back( location_s( *this ) );
}

location_s::location_s( const config_s& serverconf ) {
	alias			= "/"; // mandatory
	root			= serverconf.root; // mandatory

	// allow > may should be removed later ( since using the conf file )
	allow.push_back( GET );
	allow.push_back( POST );
	allow.push_back( DELETE );	

	index_auto		= FALSE;
}

// config_s::config_s( void ) {
// 	location		= "/";
// 	root			= "html";

// 	atidx			= FALSE;
// 	sizeBodyMax		= 1000;

// 	allow.insert( std::make_pair( GET, TRUE ) );
// 	allow.insert( std::make_pair( POST, TRUE ) );
// 	allow.insert( std::make_pair( DELETE, FALSE ) );
// }