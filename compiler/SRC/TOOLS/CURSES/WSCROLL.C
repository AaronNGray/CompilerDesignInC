/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

/*----------------------------------------------------------
 * Scroll the window if scrolling is enabled. Return 1 if we scrolled. (I'm
 * not sure if the UNIX function returns 1 on a scroll but it's convenient to
 * do it here. Don't assume anything about the return value if you're porting
 * to UNIX. Wscroll() is not a curses function. It lets you specify a scroll
 * amount and direction (scroll down by -amt if amt is negative); scroll()
 * is a macro that evaluates to a wscroll call with an amt of 1. Note that the
 * UNIX curses gets very confused when you scroll explicitly (using scroll()).
 * In particular, it doesn't clear the bottom line after a scroll but it thinks
 * that it has. Therefore, when you try to clear the bottom line, it thinks that
 * there's nothing there to clear and ignores your wclrtoeol() commands. Same
 * thing happens when you try to print spaces to the bottom line; it thinks
 * that spaces are already there and does nothing. You have to fill the bottom
 * line with non-space characters of some sort, and then erase it.
 */

wscroll(win, amt)
WINDOW	*win;
int amt;
{
    if( win->scroll_ok )
	doscroll( win->x_org, win->x_org + (win->x_size-1),
		  win->y_org, win->y_org + (win->y_size-1), amt, win->attrib );

    return win->scroll_ok ;
}
