#include "HTTP.hpp"
#include "Client.hpp"

#include "Transaction.hpp"

/*	HTTP MESSAGE FORMMAT

	HTTP-message   = start-line CRLF
                   *( field-line CRLF )
                   CRLF
                   [ message-body ]

	request-line = method SP request-target SP HTTP-version
	status-line = HTTP-version SP status-code SP [ reason-phrase ]

	field-line = field-name ":" OWS field-value OWS
*/

Transaction::Transaction( Client& client ): _cl( client ), _rqst( client ) {
	log( "HTTP\t: constructing Transaction" );

	_setTransferEnc();
	_validRequest();

	if ( _invokeCGI( _rqst, _cl.subprocs ) )
		CGI::proceed( _rqst, _cl.subprocs );
}

void
Transaction::act( void ) {
	_rspn.act( _rqst );
	build( _rspn, _cl.out );
}

// void
// Transaction::actCGI( void ) {
// 	if ( !_cl.in.chunk )
// 		CGI::writeTo ( _cl.subprocs, _cl.in.body.str().c_str(), _cl.in.body.str().size() );
// 	close( _cl.subprocs.fd[W] );
// }

void
Transaction::_setTransferEnc( void ) {
	if ( _rqst.line().method == POST ) {
		if ( _rqst.header().transfer_encoding == TE_CHUNKED )
			_cl.in.chunk = TRUE;
		else
			_cl.in.body_size = _rqst.header().content_length;
	}
}

void
Transaction::_validRequest( void ) {
	if ( _rqst.header().transfer_encoding == TE_UNKNOWN )
		throw errstat_t( 501, err_msg[TE_NOT_IMPLEMENTED] );

	if ( !getInfo( _rqst.line().uri, _rqst.info ) ) {
		if ( errno == 2 ) throw errstat_t( 404, err_msg[SOURCE_NOT_FOUND] );
		else throw errstat_t( 500 );
	}
}

bool
Transaction::_invokeCGI( const Request& rqst, process_t& procs ) {	
	size_t	dot = rqst.line().uri.rfind( "." );
	str_t	ext;

	if ( isDir( rqst.info ) )
		return FALSE;

	if ( found( dot ) )
		ext = rqst.line().uri.substr( dot );

	if ( !ext.empty() && ext != ".cgi" && ext != ".exe" ) {
		try { procs.argv.push_back( CGI::script_bin.at( ext ) );  }
		catch( exception_t& exc ) { return FALSE; }
	}
	procs.argv.push_back( rqst.line().uri );
	return TRUE;
}


bool
Transaction::recvMsg( msg_buffer_t& in, const char* buf, ssize_t& byte_read ) {
	if ( !in.msg_done ) {
		in.msg.write( buf, byte_read );

		size_t pos_header_end = in.msg.str().find( MSG_END, in.msg_read - \
			( in.msg_read * ( in.msg_read < 3 ) + 3 * !( in.msg_read < 3 ) ) );

		if ( !found( pos_header_end ) ) in.msg_read += byte_read;
		else {
			in.msg_done			= TRUE;

			size_t body_begin	= pos_header_end - in.msg_read + 4;
			in.body_read		= byte_read - body_begin;

			if ( in.body_read ) {
				in.body.write( &buf[body_begin], in.body_read );

				// in.msg.seekg( pos_header_end + 4 );
				// in.body.write( in.msg.str().c_str(), in.body_read );
				// char body[in.body_read];
				// in.msg.getline( body, in.body_read );
				// in.msg.seekg( 0 );
				
				// in.msg.str( in.msg.str().substr( 0, pos_header_end + 3 ) );
			}

			// std::clog << "--------------- completed msg ---------------\n" << in.msg.str() << "\n\n\n";

			byte_read			= 0;
		}
	}
	return in.msg_done;
}

bool
Transaction::recvBody( msg_buffer_t& in, const process_t& procs, const char* buf, const ssize_t& byte_read ) {
	// keep FALSE untill meet the content-length or tail of chunk (0CRLFCRLF)
	if ( !in.chunk ) return _recvBodyPlain( in, procs, buf, byte_read );
	else return _recvBodyChunk( in, procs, buf, byte_read );
}

bool
Transaction::_recvBodyPlain( msg_buffer_t& in, const process_t& procs, const char* buf, const ssize_t& byte_read ) {
	in.body_read += byte_read;

	if ( !procs.pid ) {
		in.body.write( buf, byte_read );

		if ( in.body_read ) {
			osstream_t oss;
			oss << "TCP\t: body read by " << byte_read << " so far: " << in.body_read << " / " << in.body_size << std::endl;
			log( oss.str() );
		}
	}

	else {
		if ( byte_read == 0 && in.body_read )
			CGI::writeTo( procs, in.body.str().c_str(), in.body_read );
		else
			CGI::writeTo( procs, buf, byte_read );
	}
	return in.body_size == in.body_read;
}

bool
Transaction::_recvBodyChunk( msg_buffer_t& in, const process_t& procs, const char* buf, const ssize_t& byte_read ) {
	// In case of following read for chunk head
	if ( in.incomplete ) {
		std::clog << "recvBodyChunk - incomplete\n";
		// Handle following read is fail, 
		// if ( byte_read != in.next_read ) {
		// 	in.next_read -= byte_read;
		// }

		if ( !procs.pid ) in.body.write( buf, byte_read - SIZE_CRLF );
		else CGI::writeTo( procs, buf, byte_read - SIZE_CRLF );

		in.incomplete	= FALSE;
		in.next_read	= SIZE_BUFF;
	}

	// In case of first body receiving right after receving message done
	if ( !byte_read && in.body_read ) {
		std::clog << "recvBodyChunk - right after receving message done\n";
		char buf_temp[in.body_read];

		in.body.read( buf_temp, in.body_read );
		in.body.str( "" );

		buf = buf_temp;

		size_t		hex;
		char		data;
		sstream_t	chunk( buf );

		while ( chunk.str().size() ) {
			//  Get chunk head
			chunk >> std::hex >> in.chunk_size >> std::ws;

			// If chunk size is0, the message is end
			if ( !in.chunk_size ) return TRUE;
			if ( in.chunk_size > 0xf ) throw errstat_t( 400, err_msg[CHUNK_EXCEED_HEX] );

			// Write body from chunk 
			hex = in.chunk_size;
			while ( hex && chunk.get( data ) ) {
				if ( !procs.pid ) in.body.write( &data, 1 );
				else CGI::writeTo( procs, &data, 1 );

				hex--;
			}

			// If unread byte is left, set the next_read
			// as rest of byte and read it following read 
			if ( hex ) {
				in.incomplete	= TRUE;
				in.next_read	= hex + SIZE_CRLF;
				return FALSE;
			}

			if ( chunk.peek() != CR ) throw errstat_t( 400, err_msg[CHUNK_EXCEED_HEX] );

			// If writing body is done well, discard the CRLF and
			// set the next_read with size of next chunk line head
			chunk >> std::ws;
		}
	}

	// In case of getting the chunk head
	else {
		std::clog << "recvBodyChunk - set chunk head\n";

		sstream_t	chunk( buf );

		chunk >> std::hex >> in.chunk_size >> std::ws;
		
		if ( !in.chunk_size ) return TRUE;

		in.next_read	= in.chunk_size;
		in.incomplete	= TRUE;
	}

	return FALSE;
}


void
Transaction::build( const Response& rspn, msg_buffer_t& out ) {
	_buildLine( rspn, out.msg );
	_buildHeader( rspn, out.msg );
	if ( rspn.body() )
		_buildBody( rspn, out );
}

void
Transaction::_buildLine( const Response& rspn, sstream_t& out_msg ) {
	map_uint_str_t::iterator iter = HTTP::key.status.find( rspn.line().status );

	out_msg <<
	HTTP::http.signature << '/' <<
	HTTP::http.version.at( static_cast<size_t>( rspn.line().version ) ) << SP <<
	iter->first << SP <<
	iter->second << CRLF;
}

void
Transaction::_buildHeader( const Response& rspn, sstream_t& out_msg ) {
	for ( vec_uint_t::const_iterator iter = rspn.header().list.begin();
		iter != rspn.header().list.end(); ++iter ) {
		_buildHeaderName( *iter, out_msg );
		_buildHeaderValue( rspn.header(), *iter, out_msg );
	}
	out_msg << CRLF;
}

void
Transaction::_buildHeaderName( uint_t id, sstream_t& out_msg ) {
	out_msg << HTTP::key.header_out.at( id ) << ": ";
}

void
Transaction::_buildHeaderValue( const response_header_t& header, uint_t id, sstream_t& out_msg ) {
	switch( id ) {
		case OUT_SERVER			: out_msg << header.server; break;
		case OUT_DATE			: break;
		case OUT_CONNECTION		: out_msg << str_connection[header.connection]; break;
		case OUT_TRANSFER_ENC	: out_msg << HTTP::http.encoding.at( header.transfer_encoding ); break;
		case OUT_CONTENT_LEN	: out_msg << header.content_length; break;
		case OUT_CONTENT_TYPE	: out_msg << header.content_type; break;
		case OUT_LOCATION		: out_msg << header.location; break;

		case OUT_ALLOW			: {
			vec_uint_t::const_iterator iter = header.allow.begin();

			while ( iter != header.allow.end() ) {
				out_msg << HTTP::http.method.at( *iter );
				if ( ++iter != header.allow.end() ) out_msg << ", ";
			}
			break;
		}
	}
	out_msg << CRLF;
}

void
Transaction::_buildBody( const Response& rspn, msg_buffer_t& out ) {
	out.body << rspn.body().rdbuf();
}
