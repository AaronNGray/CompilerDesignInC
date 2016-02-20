/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

void box( win, vert, horiz )
WINDOW	*win;
int vert, horiz;
{
    /* Draws a box in the outermost characters of the window using vert for
     * the vertical characters and horiz for the horizontal ones. I've
     * extended this function to support the IBM box-drawing characters. That
     * is, if IBM box-drawing characters are specified for vert and horiz,
     * box() will use the correct box-drawing characters in the corners. These
     * are defined in * box.h as:
     *
     *	    HORIZ   (0xc4)  single horizontal line
     *	    D_HORIZ (0xcd)  double horizontal line.
     *	    VERT    (0xb3)  single vertical line
     *	    D_VERT  (0xba)  double vertical line.
     */

    int	    i, nrows;
    int	    ul, ur, ll, lr;
    int	    oscroll, owrap, oy, ox;

    getyx( win, oy, ox );
    oscroll        = win->scroll_ok;  /* Disable scrolling and line wrap      */
    owrap          = win->wrap_ok;    /* in case window uses the whole screen */
    win->scroll_ok = 0;
    win->wrap_ok   = 0;

    if( !((horiz==HORIZ || horiz==D_HORIZ) && (vert ==VERT || vert ==D_VERT)) )
	    ul = ur = ll = lr = vert ;
    else
    {
	if( vert == VERT )
	{
	    if(horiz==HORIZ)
		ul=UL,	  ur=UR,    ll=LL,    lr=LR;
	    else
		ul=HD_UL, ur=HD_UR, ll=HD_LL, lr=HD_LR;
	}
	else
	{
	    if( horiz == HORIZ )
		ul=VD_UL, ur=VD_UR, ll=VD_LL, lr=VD_LR;
	    else
		ul=D_UL,  ur=D_UR,  ll=D_LL,  lr=D_LR;
	}
    }

    wmove ( win, 0, 0 );
    waddch( win, ul   );		    /* Draw the top line */

    for( i = win->x_size-2; --i >= 0 ; )
	waddch( win, horiz );

    waddch( win, ur );
    nrows = win->y_size - 2 ;		   /* Draw the two sides */

    i = 1 ;
    while( --nrows >= 0 )
    {
	wmove ( win, i, 0 );
	waddch( win, vert );
	wmove ( win, i++, win->x_size - 1 );
	waddch( win, vert );
    }

    wmove ( win, i, 0 );		   /* Draw the bottom line */
    waddch( win, ll   );

    for( i = win->x_size-2; --i >= 0 ; )
	    waddch( win, horiz );

    waddch( win, lr );
    wmove ( win, oy, ox );
    win->scroll_ok = oscroll ;
    win->wrap_ok   = owrap ;
}
