#ifndef LOG_HPP
# define LOG_HPP

# include "common.hpp"

# include <ctime>
# include <filesystem>

namespace log {
	extern const std::time_t	begin;
	extern File					history;

	std::string	logFname( void );
	std::string	strTime( void );

	void		timestamp( void );
	void		print( const str_t& );

	void		printVec( vec_str_t&, const str_t );
}

#endif