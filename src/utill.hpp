#ifndef UTILL_HPP
# define UTILL_HPP

# include "structure.hpp"

template<typename Container, typename Target> typename Container::iterator
lookup( Container& obj, Target token ) { return std::find( obj.begin(), obj.end(), token ); }

template<typename Container, typename Target> ssize_t
distance( Container& obj, Target token ) {
	typename Container::iterator	iter = lookup( obj, token );

	if ( iter != obj.end() ) return std::distance( obj.begin(), iter );
	else return -1;
}

char*		dupStreamBuf( std::ios&, size_t& );
const char*	dupStreamBuf( const std::ios&, size_t& );
// char*		dupStreamBuf( const osstream_t& );

#endif