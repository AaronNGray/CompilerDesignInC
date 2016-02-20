/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

WINDOW	*showwin( win )
WINDOW	*win;
{
    /* Make a previously hidden window visible again. Return NULL and do
     * nothing if the window wasn't hidden, otherwise return the win argument.
     */

    SBUF *image;

    if( !win->hidden )
	return( NULL );

    image = savescr(((SBUF*)(win->image))->left, ((SBUF*)(win->image))->right,
    		    ((SBUF*)(win->image))->top,  ((SBUF*)(win->image))->bottom);

    restore( (SBUF *) win->image );
    freescr( (SBUF *) win->image );
    win->image  = image;
    win->hidden = 0;

    /* Move the cursor to compensate for windows that were moved while they
     * were hidden.
     */
    cmove( win->y_org + win->row, win->x_org + win->col );
    return( win );
}
