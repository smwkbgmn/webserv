#include "Transaction.hpp"

#include "HTTP.hpp"
#include "Client.hpp"

/*	HTTP MESSAGE FORMMAT

	HTTP-message   = start-line CRLF
                   *( field-line CRLF )
                   CRLF
                   [ message-body ]

	request-line = method SP request-target SP HTTP-version
	status-line = HTTP-version SP status-code SP [ reason-phrase ]

	field-line = field-name ":" OWS field-value OWS
*/

/* ACCESS */
const config_t& Transaction::config( void ) { return _rqst.config(); }

connection_e
Transaction::connection( void ) {
	if ( _rqst.header().connection == CN_CLOSE ||
		_rspn.header().connection == CN_CLOSE ) return CN_CLOSE;
	else return CN_KEEP_ALIVE;
}

/* INTANTIATE */
Transaction::Transaction( Client& cl ): _cl( cl ), _rqst( cl ) {
	_setBodyEnd();
}

void
Transaction::_setBodyEnd( void ) {
	if ( _rqst.header().transfer_encoding == TE_CHUNKED ) _cl.in.chunk = TRUE;
	else _cl.in.body_size = _rqst.header().content_length;
}

/* METHOD - checkTarget:
Confirm state of target source before proceed */
void
Transaction::checkTarget( void ) {
	if ( !getInfo( _rqst.line().uri, _rqst.info ) && _rqst.location().rewrite.empty() ) {
		if ( errno == 2 ) throw errstat_t( 404, err_msg[SOURCE_NOT_FOUND] );
		if ( errno == 20 ) throw errstat_t( 404, err_msg[SOURCE_NOT_DIR] );
		throw errstat_t( 500 );
	}
}

/* METHOD - checkCGI:
Confirm if the CGI request has received and be ready for CGI proceed */
void
Transaction::checkCGI( Kqueue& kq ) {
	if ( _invokeCGI( _rqst, _cl.subproc ) ) {
		CGI::detach( _rqst, _cl.subproc );

		if ( _cl.subproc.pid ) { CGI::proceedParent( _cl.subproc.pid, _cl.sock(), kq ); }
		else { CGI::proceedChild( _rqst, _cl.subproc ); }
	}
}

bool
Transaction::_invokeCGI( const Request& rqst, process_t& procs ) {
	if ( !rqst.location().cgi || isDir( rqst.info ) || !isExist( rqst.line().uri ) ) return FALSE;

	size_t	dot = rqst.line().uri.rfind( '.' );
	str_t	ext;

	if ( found( dot ) ) ext = rqst.line().uri.substr( dot + 1 );
	if ( ext.empty() ) return FALSE;

	if ( ext != "cgi" && ext != "exe" ) {
		try { procs.argv.push_back( CGI::script_bin.at( ext ) );  }
		catch( exception_t& exc ) { return FALSE; }
	}

	procs.argv.push_back( rqst.line().uri );
	return TRUE;
}

/* METHOD - act: do the requested method and build plain response message */
void
Transaction::act( void ) {
	_rspn.act( _rqst );

	build( _rspn, _cl.out );
}

/*	
	METHOD -
	takeHead: store request line and header field at stream buffer
	then determine if the receiving message is done

	takeBody: store contetnts at steram buffer and
	determine if the receving body contents is done
*/

bool
Transaction::takeHead( message_t& in, char* buf, ssize_t& byte_read ) {
	if ( !in.head_done ) {
		in.head.write( buf, byte_read );

		size_t pos_header_end = in.head.str().find( MSG_END, in.head_read - \
			( in.head_read * ( in.head_read < 3 ) + 3 * !( in.head_read < 3 ) ) );

		if ( !found( pos_header_end ) ) in.head_read += byte_read;
		else {
			in.head_done = TRUE;

			size_t body_begin = pos_header_end - in.head_read + SIZE_MSG_END;
			byte_read -= body_begin;

			if ( byte_read )
				strncpy( buf, &buf[body_begin], byte_read );
		}
	}
	return in.head_done;
}

bool
Transaction::takeBody( message_t& in, const process_t& procs, const char* buf, const ssize_t& byte_read ) {
	if ( !in.chunk ) return _recvBodyPlain( in, procs, buf, byte_read );
	else {
		if ( byte_read == 0 && in.body_read ) return _recvBodyChunkPredata( in, procs ); 	
		return _recvBodyChunk( in, procs, buf );
	}
	/* Keep FALSE unless meet the size of content-length or tail of chunk (0CRLFCRLF) */
}

bool
Transaction::_recvBodyPlain( message_t& in, const process_t& procs, const char* buf, const ssize_t& byte_read ) {
	in.body_read += byte_read;

	if ( !procs.pid )
		in.body.write( buf, byte_read );

	else if ( !dead( procs ) ) {
		if ( byte_read == 0 && in.body_read )
			CGI::write( procs, in.body.str().c_str(), in.body_read );
		else
			CGI::write( procs, buf, byte_read );
	}
	else return TRUE;

	if ( in.body_read ) {
		osstream_t oss;

		oss << "body read by " << byte_read <<
		" (so far " << in.body_read << " / " << in.body_size << ")";
		log::print( oss.str() );
	}

	return in.body_size == in.body_read;
}

bool
Transaction::_recvBodyChunk( message_t& in, const process_t& procs, const char* buf ) {
	isstream_t	iss( buf );

	if ( in.incomplete && _recvBodyChunkIncomplete( in, procs, iss ) ) return TRUE;
	return _recvBodyChunkData( in, procs, iss );
}

bool
Transaction::_recvBodyChunkData( message_t& in, const process_t& procs, isstream_t& iss ) {
	char		data[SIZE_BUFF_CHUNK];
	
	ssize_t		frac = 1;
	ssize_t		left;

	while ( frac ) {
		iss >> std::hex >> frac >> std::ws;

		if ( iss.fail() || frac > SIZE_BUFF_CHUNK ) throw errstat_t( 400 );

		left = streamsize( iss );
		if ( left < frac + SIZE_CRLF ) in.incomplete = frac + SIZE_CRLF - left;
		if ( left < frac ) frac = left;

		iss.read( data, frac );
		if ( !procs.pid ) in.body.write( data, frac );
		else if ( !dead( procs ) ) CGI::write( procs, data, frac );
		else return TRUE;

		if ( in.incomplete ) break;

		iss >> std::ws;
	}
	return frac == 0;
}

bool
Transaction::_recvBodyChunkPredata( message_t& in, const process_t& procs ) {
	isstream_t	iss( in.body.str() );

	in.body.str( "" );
	in.body.clear();

	return _recvBodyChunkData( in, procs, iss );
}

bool
Transaction::_recvBodyChunkIncomplete( message_t& in, const process_t& procs, isstream_t& iss ) {
	char data[SIZE_BUFF_CHUNK];

	iss.read( data, in.incomplete );
	if ( !procs.pid ) in.body.write( data, in.incomplete );
	else if ( !dead( procs ) ) CGI::write( procs, data, in.incomplete );
	else return TRUE;

	iss >> std::ws;
	in.incomplete = 0;

	return FALSE;
}

/* METHOD - build: build response message base with response object */
void
Transaction::build( const Response& rspn, message_t& out ) {
	_buildLine( rspn, out.head );
	_buildHeader( rspn, out.head );

	if ( rspn.body() ) _buildBody( rspn, out );
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
		case OUT_DATE			: out_msg << timeToStr( header.date ); break;
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
		case OUT_SET_COOKIE		: out_msg << header.cookie; break;
	}
	out_msg << CRLF;
}

void
Transaction::_buildBody( const Response& rspn, message_t& out ) {
	out.body << rspn.body().rdbuf();
}

void
Transaction::buildError( const uint_t& status, Client& cl ) {
	cl.out.reset();

	if ( cl.trans ) build( Response( status, cl.trans->config() ), cl.out );
	else build( Response( status, cl.server().config().at(DEFAULT) ), cl.out );
}

/* STRUCT */
message_s::message_s() {
	reset();
}

void message_s::reset() {
	head.str("");
	head.clear();

	head_done	= false;
	head_read	= 0;
	
	body.str("");
	body.clear();

	body_size	= 0;
	body_read 	= 0;

	chunk		= false;
	incomplete	= 0;
}

bool message_s::empty() {
	return head.peek() == EOF && body.peek() == EOF;
}