/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

void delwin( win )
WINDOW	*win;
{
    /* Copy the saved image back onto the screen and free the memory used for
     * the buffer.
     */

    if( win->image )
    {
	restore ( (SBUF *) win->image );
	freescr ( (SBUF *) win->image );
    }
    free( win );
}
