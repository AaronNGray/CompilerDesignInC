/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include <tools/compiler.h>

/* FPUTSTR.C: Print a string with control characters mapped to readable strings.
 */

void	fputstr( str, maxlen, stream )
char	*str;
int	maxlen;
FILE	*stream;
{
    char *s;

    while( *str  &&  maxlen >= 0 )
    {
	s = bin_to_ascii( *str++, 1 );
	while( *s && --maxlen >= 0 )
	    putc( *s++ , stream );
    }
}
