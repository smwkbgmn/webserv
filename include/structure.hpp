#ifndef STRUCTURE_HPP
# define STRUCTURE_HPP

# include <iostream>
# include <sstream>

# include <string>
# include <vector>
# include <map>

typedef int								socket_t;
typedef unsigned int					uint_t;
typedef unsigned int					bits_t;
typedef std::string						str_t;

/* I/O */
typedef std::ostringstream				osstream_t;
typedef std::istringstream				isstream_t;

/* vector */
typedef std::vector<uint_t>				vec_uint_t;
typedef std::vector<str_t>				vec_str_t;
typedef vec_str_t::iterator				vec_str_iter_t;

/* map */
typedef std::map<str_t, str_t>			map_str_str_t;
typedef std::map<uint_t, str_t>			map_uint_str_t;

# include "File.hpp"
# include "filter.hpp"
# include "error.hpp"

#endif
