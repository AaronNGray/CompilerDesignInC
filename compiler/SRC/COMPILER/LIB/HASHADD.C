/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/debug.h>
#include <tools/hash.h>
/*----------------------------------------------------------------------
 * Hash function for use with the functions in hash.c. Just adds together
 * characters in the name.
 */

unsigned hash_add( name )
unsigned char	*name;
{
    unsigned h ;

    for( h = 0;  *name ;  h += *name++ )
	    ;
    return h;
}
