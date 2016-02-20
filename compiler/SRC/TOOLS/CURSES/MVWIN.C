/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

/* Move a window to new absolute position. This routine will behave in one of
 * two ways, depending on the value of LIKE_UNIX when the file was compiled.
 * If the #define has a true value, then mvwin() returns ERR and does nothing
 * if the new coordinates would move the window past the edge of the screen.
 * If LIKE_UNIX is false, ERR is still returned but the window is moved flush
 * with the right edge if it's not already there. ERR says that the window is
 * now flush with the edge of the screen. In both instances, negative
 * coordinates are silently set to 0.
 */

#define LIKE_UNIX 0

#if ( LIKE_UNIX )
#	undef  UNIX
#	define UNIX(x) x
#	define DOS(x)
#else
#	undef  UNIX
#	define UNIX(x)
#	define DOS(x) x
#endif

mvwin( win, y, x )
WINDOW	*win;
int	y, x;
{
    int old_x, old_y, xsize, ysize, delta_x, delta_y, visible;
    SBUF *image;

    if( win == stdscr )		/* Can't move stdscr without it going   */
	return ERR;		/* off the screen.		      */

    /* Get the actual dimensions of the window: compensate for a border if the
     * window is boxed.
     */

    old_x = win->x_org  -  win->boxed ;
    old_y = win->y_org  -  win->boxed ;
    xsize = win->x_size + (win->boxed * 2);
    ysize = win->y_size + (win->boxed * 2);

    /* Constrain x and y so that the window can't go off the screen */

    x = max( 0, x );
    y = max( 0, y );
    if( x + xsize > 80 )
    {
	UNIX( return ERR;	)
	DOS ( x = 80 - xsize;	)
    }
    if( y + ysize > 25 )
    {
	UNIX( return ERR;	)
	DOS ( y = 25 - ysize;	)
    }

    delta_x = x - old_x;			/* Adjust coordinates. */
    delta_y = y - old_y;

    if( delta_y == 0 && delta_x == 0 )
	    return ERR;

    if( visible = !win->hidden )
	    hidewin( win );

    win->y_org    += delta_y;
    win->x_org    += delta_x;
    image	  =  (SBUF *) win->image;
    image->top    += delta_y;
    image->bottom += delta_y;
    image->left   += delta_x;
    image->right  += delta_x;

    if( visible )
	showwin( win );
    return( OK );
}
