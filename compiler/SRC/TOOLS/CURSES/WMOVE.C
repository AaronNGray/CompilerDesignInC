/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

void wmove( win, y, x )
WINDOW	*win;
int y, x;
{
    /* Go to window-relative position. You can't go outside the window. */

    cmove( win->y_org + (win->row = min(y,win->y_size-1)) ,
	   win->x_org + (win->col = min(x,win->x_size-1)) );
}
