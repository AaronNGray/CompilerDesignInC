/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/vbios.h>

void	vb_puts( str, move_cur )
char	*str;
int	move_cur;
{
    /* Write a string to the screen in TTY mode. If move_cur is true the cursor
     * is left at the end of string. If not the cursor will be restored to its
     * original position (before the write).
     */

    int	posn;

    if( !move_cur )
	posn = VB_GETCUR();

    while( *str )
	VB_PUTCHAR( *str++ );

    if( !move_cur )
	VB_SETCUR( posn );
}
