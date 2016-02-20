/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

WINDOW	*hidewin( win )
WINDOW	*win;
{
    /* Hide a window. Return NULL and do nothing if the image wasn't saved
     * originally or if it's already hidden, otherwise hide the window and
     * return the win argument. You may not write to a hidden window.
     */

    SBUF *image;

    if( !win->image || win->hidden )
	return NULL;

    image = savescr(((SBUF*)(win->image))->left,((SBUF*)(win->image))->right,
    		    ((SBUF*)(win->image))->top, ((SBUF*)(win->image))->bottom );

    restore( (SBUF *) win->image );
    freescr( (SBUF *) win->image );
    win->image  = image;
    win->hidden = 1;
    return( win );
}
