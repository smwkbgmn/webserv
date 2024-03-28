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

enum methodID {
	GET,
	POST,
	DELETE
};

enum versionID {
	VERSION_9,
	VERSION_10,
	VERSION_11,
	VERSION_20
};

typedef struct {
	methodID	method;
	str_t		target;
	versionID	version;
}	request_line_t;

typedef struct {
	versionID	version;
	status_t	status;
}	response_line_t;

#endif