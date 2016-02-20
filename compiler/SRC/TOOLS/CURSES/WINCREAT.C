/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"
#include <tools/box.h>

/*--------------------------------------------------------
 * Window creation functions.
 * Standard Functions:
 *
 *    WINDOW	*newwin( lines, cols, begin_y, begin_x )
 *		creates a window
 *
 * Nonstandard Functions:
 *
 *    save()  Area under all new windows is saved (default)
 *  nosave()  Area under all new windows is not saved
 *
 *   boxed()  		Window is boxed automatically.
 * unboxed()  		Window is not boxed (default)
 * def_ground(f,b)	Set default foreground color to f, and background color
 *			to b.
 *--------------------------------------------------------
 */

PRIVATE int Save   = 1; 	/* Save image when window created	*/
PRIVATE int Box	   = 0;	 	/* Windows are boxed		  	*/
PRIVATE int Attrib = NORMAL;	/* Default character attribute byte	*/

void save    () {  Save = 1;	}
void nosave  () {  Save = 0;	}
void boxed   () {  Box  = 1;	}
void unboxed () {  Box  = 0;	}

void def_ground(f, b)
int f, b;
{
    Attrib = (f & 0x7f) | ((b & 0x7f) << 4);
}

/*--------------------------------------------------------*/

WINDOW	*newwin( lines, cols, begin_y, begin_x )
int	cols;	  /* Horizontal size (including border)	  */
int	lines;	  /* Vertical size  (including border)	  */
int	begin_y;  /* X coordinate of upper-left corner	  */
int	begin_x;  /* Y coordinate of upper-left corner	  */
{
    WINDOW	*win;

    if( !(win = (WINDOW *) malloc( sizeof(WINDOW) )) )
    {
	fprintf(stderr,"Internal error (newwin): Out of memory\n");
	exit(1);
    }

    if( cols > 80 )
    {
	cols    = 80;
	begin_x = 0;
    }
    else if( begin_x + cols > 80 )
	begin_x = 80 - cols;

    if( lines > 25 )
    {
	lines   = 25;
	begin_y = 0;
    }
    else if( begin_y + lines > 25 )
	begin_x = 25 - cols;

    win->x_org	    = begin_x ;
    win->y_org	    = begin_y ;
    win->x_size	    = cols    ;
    win->y_size	    = lines   ;
    win->row	    = 0 ;
    win->col	    = 0 ;
    win->scroll_ok  = 0 ;
    win->wrap_ok    = 1 ;
    win->boxed      = 0 ;
    win->hidden     = 0 ;
    win->attrib	    = Attrib ;
    win->image      = !Save ? NULL : savescr( begin_x, begin_x + (cols  - 1) ,
					      begin_y, begin_y + (lines - 1) );
    werase(win);

    if( Box )				/* Must be done last */
    {
        box( win, VERT, HORIZ );	/* Box it first		   */
	win->boxed   = 1;
	win->x_size -= 2;		/* Then reduce window size */
	win->y_size -= 2;		/* so that the box won't   */
	win->x_org  += 1;		/* be overwritten.	   */
	win->y_org  += 1;

        cmove( win->y_org, win->x_org );
    }
    return win;
}
