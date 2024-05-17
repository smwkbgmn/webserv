#ifndef TRANSACTION_HPP
# define TRANSACTION_HPP

# include "Request.hpp"
# include "Response.hpp"

const char hexdigt[17] = "0123456789ABCDEF";

class Transaction {
	public:
		Transaction( Client& );

		static bool		recvMsg( msg_buffer_t&, const char*, ssize_t& );
		static bool		recvBody( msg_buffer_t&, const process_t&, const char*, const ssize_t& );

		static void		build( const Response&, msg_buffer_t& );

		void			act( void );
		// void			actCGI( void );

	private:
		Client&			_cl;

		Request			_rqst;
		Response		_rspn;

		static bool		_recvBodyPlain( msg_buffer_t&, const process_t&, const char*, const ssize_t& );
		static bool		_recvBodyChunk( msg_buffer_t&, const process_t&, const char*, const ssize_t& );

		static void		_buildLine( const Response&, sstream_t& );
		static void		_buildHeader( const Response&, sstream_t& );
		static void		_buildHeaderName( uint_t, sstream_t& );
		static void		_buildHeaderValue( const response_header_t&, uint_t, sstream_t& );
		static void		_buildBody( const Response&, msg_buffer_t& );
		
		void			_setTransferEnc( void );
		void			_validRequest( void );
		bool			_invokeCGI( const Request&, process_t& );
};


# include "Client.hpp"

#endif