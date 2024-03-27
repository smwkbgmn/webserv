#ifndef PARSE_HPP
# define PARSE_HPP

# include "structure.hpp"

// # define FALSE		0
// # define TRUE		1

/* VALUE */
# define NONE				0

// # define HTTP_VERSION_9 	9
// # define HTTP_VERSION_10	1000
// # define HTTP_VERSION_11	1001
// # define HTTP_VERSION_20	2000

/* TOKEN */
# define LF					'\n'
# define CR					'\r'
# define CRLF				"\r\n"
# define SP					' '

/* TRANSACTION */
# define CNT_METHOD			3

enum 

const str_t		httpName	= "HTTP";
const vec_str_t	httpVersion	= {
	"0.9",
	"1.0",
	"1.1",
	"2.0"
};

enum methodID {
	GET,
	POST,
	DELETE
};

const str_t	methodName[CNT_METHOD] = {
	"GET",
	"POST",
	"DELETE"
};


#endif 