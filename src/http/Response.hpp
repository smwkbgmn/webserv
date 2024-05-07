#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "utill.hpp"

# include "Request.hpp"

class Response {
	public:
		Response( const Request& );
		Response( const Client&, const uint_t& ); // When fail to construct Request
		~Response( void );

		const response_line_t&		line( void ) const;
		const response_header_t&	header( void ) const ;
		const char*					body( void ) const;

	private:
		response_line_t				_line;
		response_header_t			_header;
		char*						_body;

		void						_errpage( const uint_t&, const config_t& );
		void						_errpageBuild( const uint_t& );
		void						_mime( const str_t&, str_t&, const str_t& );
};

# include "HTTP.hpp"

#endif