#ifndef TRANSACTION_HPP
# define TRANSACTION_HPP

# include "Request.hpp"
# include "Response.hpp"

# define SIZE_BUFF		1024
# define SIZE_BUFF_RECV 2048

typedef struct message_s{
	message_s();
	
	void	reset();
	
	sstream_t	head;
	ssize_t		head_read;
	bool		head_done;

	sstream_t	body;
	ssize_t		body_read;
	ssize_t		body_size;

	bool		chunk;
	size_t		incomplete;
}	message_t;

static const char hexdigit[17] = "0123456789ABCDEF";

class Transaction {
	public:
		Transaction( Client& );

		const config_t&		config( void );
		connection_e		connection( void );			

		static bool			recvHead( message_t&, char*, ssize_t& );
		static bool			recvBody( message_t&, const process_t&, const char*, const ssize_t& );

		static void			build( const Response&, message_t& );
		static void			buildError( const uint_t&, Client& );

		void				act( void );

	private:
		Client&				_cl;

		Request				_rqst;
		Response			_rspn;

		static bool			_recvBodyPlain( message_t&, const process_t&, const char*, const ssize_t& );
		static bool			_recvBodyChunk( message_t&, const process_t&, const char* );
		static bool			_recvBodyChunkData( message_t&, const process_t&, isstream_t& );
		static bool			_recvBodyChunkPredata( message_t&, const process_t& );
		static bool			_recvBodyChunkIncomplete( message_t&, const process_t&, isstream_t& );

		static void			_buildLine( const Response&, sstream_t& );
		static void			_buildHeader( const Response&, sstream_t& );
		static void			_buildHeaderName( uint_t, sstream_t& );
		static void			_buildHeaderValue( const response_header_t&, uint_t, sstream_t& );
		static void			_buildBody( const Response&, message_t& );
		
		void				_validRequest( void );
		void				_setBodyEnd( void );
		bool				_invokeCGI( const Request&, process_t& );
};

# include "Client.hpp"

#endif