/*@A (C) 1992 Allen I. Holub                                                */
#include "video.h"
#include <tools/termlib.h>

void dv_puts( str, move )
char	*str;
int	move;
{
    /* Write string to screen, moving cursor to end of string only if move is
     * true. Use normal attributes.
     */

    int orow, ocol;

    dv_getyx( &orow, &ocol );

    while( *str )
	dv_putc( *str++, NORMAL );

    if( !move )
	dv_ctoyx( orow, ocol );
}
