#include "utill.hpp"

/* BUFFER */
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

/* FILE INFO */
bool
getInfo( const str_t& target, fstat_t& info ) {
	return stat( target.c_str(), &info ) != ERROR;
}

bool
isExist( const str_t& target ) {
	fstat_t	info;

	return stat( target.c_str(), &info ) != ERROR && !isDir( info );
}

bool isDir( const fstat_t& info ) { return S_ISDIR( info.st_mode ); }

/* ERROR PAGE */
void
errpageScript( sstream_t& page, const uint_t& status, const str_t& explanation ) {
	page <<
	"<!DOCTYPE html>\n" <<
	"<html lang=\"en\">\n" <<
	"<head>\n" <<
	"<meta charset=\"UTF-8\">\n" <<
	"<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n" <<
	"<title>HTTP Response</title>\n" <<
	"<style>\n" <<
	"body {\n" <<
	"font-family: Arial, sans-serif;\n" <<
	"margin: 0;\n" <<
	"padding: 20px;\n" <<
	"}\n" <<
	".container {\n" <<
	"max-width: 800px;\n" <<
	"margin: 50px auto;\n" <<
	"background-color: #f4f4f4;\n" <<
	"padding: 20px;\n" <<
	"border-radius: 5px;\n" <<
	"box-shadow: 0 2px 5px rgba(0, 0, 0, 0.1);\n" <<
	"overflow: auto;" <<
	"positon: relative;" <<
	"}\n" <<
	"h1 {\n" <<
	"color: #007bff;\n" <<
	"}\n" <<
	"p {\n" <<
	"margin-bottom: 20px;\n" <<
	"}\n" <<
	".image-container {" <<
	"position: absolute;" <<
	"bottom: 20px;" <<
	"right: 25px;" <<
	"}" <<
	"img {" <<
	"width: 100px;" <<
	"height: auto;" <<
	"}" <<
	"</style>\n" <<
	"</head>\n" <<
	"<body>\n" <<
	"<div class=\"container\">\n" <<
	// Modify title as the server name later
	"<h1>Webserv responsing with error state</h1>\n" <<
	"<div class=\"image-container\">" <<
    "<img src=\"favicon.ico\" alt=\"Image\">" <<
	"</div>" <<
	"<p><strong>Status Code:</strong> " << status << "</p>\n" <<
	"<p><strong>Explanation:</strong> " << explanation << "</p>\n" <<
	"</div>\n" <<
	"</body>\n" <<
	"</html>";
}
