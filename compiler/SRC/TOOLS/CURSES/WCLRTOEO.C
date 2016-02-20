/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

void wclrtoeol( win )
WINDOW	*win;
{
    /* Clear from cursor to end of line, the cursor isn't moved. The main
     * reason that this is included here is because you have to call it after
     * printing every newline in order to compensate for a bug in the real
     * curses. This bug has been corrected in my curses, so you don't have to
     * use this routine if you're not interested in portability.
     */

    clr_region( win->x_org +  win->col , win->x_org + (win->x_size - 1),
		win->y_org +  win->row , win->y_org +  win->row,
		win->attrib						);
}
