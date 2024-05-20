#include "HTTP.hpp"

http_t HTTP::http;
keys_t HTTP::key;

/* METHOD - init: assign basic HTTP info and load keys */
void
HTTP::init( void ) {
	http.signature			= "HTTP";
	http.type_unknown		= "text/plain";

	_assignVec( http.version, str_version, CNT_VERSION );
	_assignVec( http.method, str_method, CNT_METHOD );
	_assignVec( http.encoding, str_transfer_enc, CNT_ENCODING );
	_assignVec( http.connection, str_connection, CNT_CONNECTION );

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

/* METHOD - getLocationConf: get index for location_t matching with request URI */

size_t
HTTP::setConfig( const str_t& host, const vec_config_t& configs ) {
	if ( configs.size() > 1 && !host.empty() ) {
		vec_config_t::const_iterator	iter;
		size_t							idx;

		idx = 1;
		for ( iter = configs.begin() + 1; iter != configs.end(); ++iter ) {
			if ( _setConfigMatchName( host, iter->names, iter->listen ) ) return idx;
			++idx;
		}
	}
	return 0;
}

bool
HTTP::_setConfigMatchName( const str_t& host, const vec_str_t& names, const uint_t& listen ) {
	isstream_t	iss( host );

	str_t		name;
	uint_t		port;

	try {
		name = token( iss, ':' );
		iss >> std::ws >> port;

		return distance( names, name ) != NOT_FOUND &&
			port == listen;
	}
	catch ( err_t& err ) { return distance( names, host ) != NOT_FOUND; }
}

/*
	Enforce server root config > location without root block follows this
	if the server config has no root block, set default root as html
	
	Cases of use of location config:
	1. dir: starting with the slash '/'
	2. extension: starting with the dot '.' ( ^~ \ )
	3. file: other else, means without slash or dot neither ( = / )
*/

size_t
HTTP::setLocation( const path_t& uri, const vec_location_t& locations ) {
	if ( locations.size() > 1 ) {
		size_t							dot		= uri.find( '.' );

		vec_location_t::const_iterator	iter;
		size_t							idx;
		
		// Search for extension config 
		if ( found( dot ) ) {
			str_t ext = uri.substr( dot );

			idx = 1;
			for ( iter = locations.begin() + 1; iter != locations.end(); ++iter ) {
				if ( ext == iter->path ) return idx;
				++idx;
			}
		}

		// Search for location config
		idx = 1;
		for ( iter = locations.begin() + 1; iter != locations.end(); ++iter ) {
			if ( uri.find( iter->path ) == 0 ) return idx;
			++idx;
		}
	}
	return 0;
}

/* STURCT */
config_s::config_s( void ) {
	// if no server_names are given, set the default name while conf parsing
	names.push_back( "webserv.com" );

	listen			= 8080; // mandatory
	root			= "html"; // mandatory

	client_max_body	= 10240; // 10M

	/*
		The root configuration (i.e. location for "/") could be set or not and
		either case the location conf for the root MUST exist. It works as the 
		default.
	*/
	locations.push_back( location_s( *this ) );
	
	// add location for cgi-bin 
	location_s cgi_bin( *this );

	cgi_bin.path = "/cgi-bin";

	// when parsing config, if the root for location has no block,
	// inherit the server config root
	cgi_bin.root = "html";
	cgi_bin.cgi	= TRUE;

	cgi_bin.allow.push_back( GET );
	cgi_bin.allow.push_back( POST );
	cgi_bin.allow.push_back( DELETE );

	cgi_bin.index.push_back( "index.txt" );
	cgi_bin.index_auto = TRUE;
	// cgi_bin.index_auto = FALSE;

	cgi_bin.upload_path = "html/new_upload";

	locations.push_back( cgi_bin );
}

location_s::location_s( const config_s& serverconf ) {
	path			= "/"; // mandatory
	root			= serverconf.root; // mandatory

	// allow > may should be removed later ( since using the conf file )
	allow.push_back( GET );
	allow.push_back( POST );
	allow.push_back( DELETE );	

	// index_auto		= FALSE;
	index_auto		= TRUE;

	index.push_back( "index.html" );
}
