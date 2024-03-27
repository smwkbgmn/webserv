#ifndef ERROR_HPP
# define ERROR_HPP

# define ERROR -1

# include <exception>

# include <sys/errno.h>

typedef std::runtime_error	err_t;

enum errID {
	INVALID_REQUEST_LINE
};

str_t	errMsg[] = {
	"invalid request line"
};

#endif