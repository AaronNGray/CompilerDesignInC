/*@A (C) 1992 Allen I. Holub                                                */
#include <string.h>
#include <stdlib.h>
#include <tools/debug.h>	/* to map malloc() to dmalloc() if necessary */

char	*strdup( str )
char	*str;
{
    /* Save the indicated string in a malloc()ed section of static memory.
     * Return a pointer to the copy or NULL if malloc fails.
     */

    char *rptr;

    if( rptr = (char *) malloc( strlen(str) +1 ) )
	strcpy( rptr, str );

    return rptr;
}
