/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

int	winch( win )
WINDOW	*win;
{
    int	y, x, c;

    curpos( &y, &x );
    cmove( win->y_org + win->row, win->x_org + win->col );
    c = inchar();
    cmove( y, x );
    return c;
}
