#ifndef LOG_HPP
# define LOG_HPP

# include <iostream>
# include <sstream>
# include <ctime>

# include "File.hpp"
# include "error.hpp"

extern File logfile;

std::string	logFname( void );
std::string	strTime( void );

#endif