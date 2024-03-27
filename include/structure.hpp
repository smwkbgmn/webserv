#ifndef STRUCTURE_HPP
# define STRUCTURE_HPP

/* STREAM */
# include <iostream>
# include <sstream>

/* CONTAINER */
# include <vector>
# include <map>

/* UTILL */
# include <string>

struct request_line_s;
struct response_line_s;

typedef struct request_line_s			request_line_t;
typedef struct response_line_s			response_line_t;

typedef unsigned int					uint_t;
typedef unsigned int					bits_t;

typedef int								socket_t;

typedef std::istringstream				isstream_t;

typedef std::string						str_t;

typedef std::vector<str_t>				vec_str_t;
typedef vec_str_t::iterator				vec_str_iter_t;

typedef std::map<str_t, vec_str_t>		mime_t;
typedef std::map<str_t, vec_str_t>		method_t;
typedef std::map<unsigned int, str_t>	status_t;


struct request_line_s {
	methodID	method;
	str_t		target;
	uint_t		version;
};

struct response_line_s {
	uint_t		version;
	status_t	status;
};

#endif