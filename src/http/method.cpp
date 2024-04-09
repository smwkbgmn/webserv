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
*/

char*
HTTP::GET( const str_t& uri, size_t& size ) {
	try {
		File target( config.dirRoot + uri, R_BINARY );

		std::filebuf* pbuf = target.fs.rdbuf();
		size = pbuf->pubseekoff( 0, target.fs.end, target.fs.in );
		pbuf->pubseekpos( 0, target.fs.in );

		char *buf = new char[size];
		pbuf->sgetn( buf, size );
		
		return buf;
	} catch ( exception_t& exc ) { return NULL; }
}
 
void
HTTP::POST( const Request& rqst ) {
	File target( config.dirRoot + rqst.line().uri, W );

	target.fs << rqst.body();
}

bool
HTTP::DELETE( const Request& rqst ) {
	stat_t	statBuf;

	if ( stat( rqst.line().uri.c_str(), &statBuf ) != ERROR )
		return std::remove( ( config.dirRoot + rqst.line().uri ).c_str() ) == ERROR;

	return FALSE;
}
