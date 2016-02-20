/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include <tools/compiler.h>
#include <io.h>

movefile( dst, src, mode )    /* Works like copyfile() (see copyfile.c)    */
char	*dst, *src;	      /* but deletes src if the copy is successful */
char	*mode;
{
    int  rval;
    if( (rval = copyfile(dst, src, mode)) == 0 )
	unlink( src );
    return rval;
}
