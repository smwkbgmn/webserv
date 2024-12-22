#include "log.hpp"

const std::time_t	log::begin = std::time( NULL );
File				log::history( "log/" + logFname(), WRITE );

std::string
log::logFname( void ) {
	fstat_t	info;

	if ( stat( "log", &info ) == ERROR )
		mkdir( "log", 0755 );
		
	std::string	fname = log::strTime() + ".log";

	return fname;
}

std::string
log::strTime( void ) {
	std::time_t	now = std::time( NULL );

	char		buf[80];
	std::strftime( buf, sizeof( buf ), "%Y%m%d_%H%M%S", std::localtime( &now ) );
	
	return std::string( buf );
}

void
log::timestamp( void ) {
	std::time_t	now			= std::time( NULL );
	std::time_t	elapse		= now - begin;
	std::tm*	timeinfo	= std::localtime( &elapse );

	char buf[100];
	timeinfo->tm_hour -= 9;
	std::strftime( buf, 100, "[%H:%M:%S]", timeinfo );
	std::clog << buf << " ";
}

void
log::print( const str_t& msg ) {
	timestamp();
	std::clog << msg << std::endl;
}
