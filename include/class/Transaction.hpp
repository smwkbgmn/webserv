#ifndef TRANSACTION_HPP
# define TRANSACTION_HPP

# include <algorithm>

# include "HTTP.hpp"
# include "parse.hpp"
# include "error.hpp"

/*
	[Message object for HTTP]
	accept LF and CRLF either

	GET: refuse body contents
*/

class Request {
	public:
		Request( const str_t& msgRequest );
		~Request( void );

	private:
		request_line_t	_line;
		// header_t		_header;
		bits_t			_body; // Should be changed later to Transfer-encoding type

		void			_getLine( str_t );
		void			_assignMethod( str_t );
		void			_assignURI( str_t );
		void			_assignVersion( str_t );

		void			_getHeader( str_t );
		void			_getBody( str_t );

		str_t			_token( isstream_t&, char );
		
		template<typename T>
		typename T::iterator	_find( T&, str_t );
		
};

class Response {
	private:
	public:
		response_line_t	line;
		// header_t		header;
		bits_t			body;		
};

class Transaction: Request, Response {
	public:
		Transaction( const str_t& );
		~Transaction( void );

		/* Resource Mapping(Docroot) */

	private:
		Transaction( void );
		Transaction( const Transaction& );
		
		Transaction&	operator=( const Transaction& );
};

#endif