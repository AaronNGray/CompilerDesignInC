/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

void werase( win )
WINDOW	*win;
{
    clr_region( win->x_org, win->x_org + (win->x_size - 1),
	        win->y_org, win->y_org + (win->y_size - 1),
		win->attrib				 );

    cmove( win->y_org, win->x_org );
    win->row = 0;
    win->col = 0;
}
