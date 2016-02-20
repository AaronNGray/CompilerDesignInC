/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tools/debug.h>
#ifdef ANSI
#include <io.h>
#endif

#ifndef O_RDONLY
#include <fcntl.h>
#endif

#define ERR_NONE 	 0
#define ERR_DST_OPEN	-1
#define ERR_SRC_OPEN	-2
#define ERR_READ	-3
#define ERR_WRITE	-4

copyfile( dst, src, mode )
char	*dst, *src;
char	*mode;		/* "w" or "a" */
{
    /* Copy the src to the destination file, opening the destination in the
     * indicated mode. Note that the buffer size used on the first call is
     * used on subsequent calls as well. Return values are defined, above.
     * errno will hold the appropriate error code if the return value is <0.
     */

    int      	    fd_dst, fd_src;
    char     	    *buf;
    int	     	    got;
    int		    ret_val = ERR_NONE  ;
    static long     size    = 31 * 1024 ; /* must hold a value that can fit */
    					  /* in an int.			    */

    while( size > 0  &&  !(buf = malloc( (int)size )) )
	size -= 1024L;

    if( !size )		/* couldn't get a buffer, do it one byte at a time */
    {
	size = 1;
	buf  = "xx";	/* allocate a buffer implicitly */
    }

    fd_src = open(src, O_RDONLY | O_BINARY );
    fd_dst = open(dst, O_WRONLY | O_BINARY | O_CREAT |
					 (*mode=='w' ? O_TRUNC : O_APPEND),
					 S_IREAD|S_IWRITE);

    if     ( fd_src == -1 ){ ret_val = ERR_SRC_OPEN; }
    else if( fd_dst == -1 ){ ret_val = ERR_DST_OPEN; }
    else
    {
	while( (got = read(fd_src, buf, (int)size ))  >  0 )
	    if( write(fd_dst, buf, got) == -1 )
	    {
		ret_val = ERR_WRITE;
		break;
	    }

	if( got == -1 )
	    ret_val = ERR_READ;
    }

    if( fd_dst != -1 ) close ( fd_dst );
    if( fd_src != -1 ) close ( fd_src );
    if( size   > 1   ) free  ( buf    );

    return ret_val;
}
