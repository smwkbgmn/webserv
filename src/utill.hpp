#ifndef UTILL_HPP
# define UTILL_HPP

# include "structure.hpp"

template<typename Container, typename Target> typename Container::iterator
lookup( Container& obj, Target token ) { return std::find( obj.begin(), obj.end(), token ); }

template<typename Container, typename Target> typename Container::const_iterator
lookup( const Container& obj, Target token ) { return std::find( obj.begin(), obj.end(), token ); }

template<typename Container, typename Target> ssize_t
distance( Container& obj, Target token ) {
	typename Container::iterator	iter = lookup( obj, token );

	if ( iter != obj.end() ) return std::distance( obj.begin(), iter );
	else return -1;
}

/* BUFFER */
char*		dupStreamBuf( std::ios&, size_t& );
const char*	dupStreamBuf( const std::ios&, size_t& );

/* FILE INFO */
bool		getInfo( const str_t&, fstat_t& );
bool		isExist( const str_t& );
bool		isDir( const fstat_t& );

/* ERROR PAGE */
void		errpageScript( sstream_t&, const uint_t&, const str_t& );
void		autoindexScript( const path_t&, sstream_t& );

#endif