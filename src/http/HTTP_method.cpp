#include "HTTP.hpp"

/*
	#define __DARWIN_STRUCT_STAT64 { \
		dev_t		st_dev;                 [XSI] ID of device containing file
		mode_t		st_mode;                [XSI] Mode of file (see below)
		nlink_t		st_nlink;               [XSI] Number of hard links
		__darwin_ino64_t st_ino;            [XSI] File serial number
		uid_t		st_uid;                 [XSI] User ID of the file
		gid_t		st_gid;                 [XSI] Group ID of the file
		dev_t		st_rdev;                [XSI] Device ID
		__DARWIN_STRUCT_STAT64_TIMES \
		off_t		st_size;                [XSI] file size, in bytes
		blkcnt_t	st_blocks;              [XSI] blocks allocated for file
		blksize_t	st_blksize;             [XSI] optimal blocksize for I/O
		__uint32_t	st_flags;               user defined flags for file
		__uint32_t	st_gen;                 file generation number
		__int32_t	st_lspare;              RESERVED: DO NOT USE!
		__int64_t	st_qspare[2];           RESERVED: DO NOT USE!
	}

 	[XSI] The following macros shall be provided to test whether a file is
 	of the specified type.  The value m supplied to the macros is the value
 	of st_mode from a stat structure.  The macro shall evaluate to a non-zero
 	value if the test is true; 0 if the test is false.

	#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)      block special 
	#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)      char special 
	#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)      directory 
	#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)      fifo or socket 
	#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)      regular file 
	#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)      symbolic link 
	#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)     socket 
	#if !defined(_POSIX_C_SOURCE) || defined(_DARWIN_C_SOURCE)
	#define S_ISWHT(m)      (((m) & S_IFMT) == S_IFWHT)      OBSOLETE: whiteout 
	#endif
*/

// When fail to get all of default index files, set status as 403 forbidden
void
HTTP::GET( const Request& rqst, char** bufptr, size_t& size ) {
	try {
		File target( rqst.line().uri, R_BINARY );
		
		*bufptr = dupStreamBuf( target.fs, size );
	} catch ( err_t& exc ) { log( "HTTP\t: " + str_t( exc.what() ) ); throw errstat_t( 404 ); }
}
 
void
HTTP::POST( const Request& rqst, char** bufptr, size_t& size ) {
	( void )bufptr;
	( void )size;

	try {
		File target( rqst.line().uri, W );

		target.fs << rqst.body();
	} catch ( exception_t& exc ) { log( str_t( exc.what() ) ); throw errstat_t( 400 ); }
}

void
HTTP::DELETE( const Request& rqst ) {
	// stat_t	statbuf;

	// if ( stat( rqst.line().uri.c_str(), &statbuf ) != ERROR )
	if ( std::remove( rqst.line().uri.c_str() ) == ERROR )
		throw errstat_t( 404 );

	// return FALSE;
}
