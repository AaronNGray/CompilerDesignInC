/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

WINDOW	*stdscr;

void	endwin()			 /* Clean up as required */
{
    cmove( 24,0 );
}

void	initscr()
{
    /* Creates stdscr. If you want a boxed screen, call boxed() before this
     * routine. The underlying text is NOT saved. Note that the atexit call
     * insures a clean up on normal exit, but not with a Ctrl-Break, you'll
     * have to call signal() to do that.
     */

    nosave();
    init(-1);
    stdscr = newwin( 25, 80, 0, 0 );
    save();
    atexit( (void (*)(void)) endwin );
}
