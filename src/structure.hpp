#ifndef STRUCTURE_HPP
#define STRUCTURE_HPP

#include <string>

typedef int							socket_t;
typedef int							port_t;
typedef int							pipe_t;
typedef int							stat_t;

typedef unsigned int				uint_t;
typedef unsigned int				bits_t;

typedef std::string					str_t;

/*
	name_t: relative path, means when use it, should be combined with absolute path
	path_t: absolute path
	type_t: the type of mime
*/

typedef str_t						name_t;
typedef str_t						path_t;
typedef str_t						type_t;

/* UNIX */
#include <sys/stat.h>

typedef struct stat					fstat_t;

/* I/O */
#include <iostream>
#include <sstream>

typedef std::ostringstream 			osstream_t;
typedef std::istringstream 			isstream_t;

/* Container */
#include <map>
#include <vector>

typedef std::vector<uint_t>			vec_uint_t;
typedef std::vector<str_t>			vec_str_t;
typedef vec_str_t					vec_name_t;
typedef vec_str_t::iterator			vec_str_iter_t;
typedef std::vector<char*>			vec_cstr_t;

typedef std::map<str_t, type_t>		map_str_type_t;
typedef std::map<uint_t, str_t>		map_uint_str_t;

/* Struct */
typedef struct process_s {
	pid_t	pid;
	pipe_t	fd[2];
	stat_t	stat;

	process_s( void );
}	process_t;

#include "File.hpp"
#include "error.hpp"
#include "filter.hpp"

#endif
