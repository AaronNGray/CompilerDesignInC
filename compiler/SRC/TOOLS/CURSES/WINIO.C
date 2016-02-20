/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

#if ( (0 BCC(+1)) || (0 MSC(+1)) )	/* Microsoft or Borland Compilers */
#include <dos.h>			/* Prototype for bdos().	  */
#endif

/*--------------------------------------------------------
 * WINIO.C	Lowest level I/O routines:
 * 	waddch(win,c)  		wgetch(win)
 *	echo()			noecho()
 *	nl()			nonl()
 *	crmode()		nocrmode()
 *--------------------------------------------------------
 */

PRIVATE int Echo   = 1;		/* Echo enabled			  */
PRIVATE int Crmode = 0;		/* If 1, use buffered input	  */
PRIVATE int Nl	   = 1;		/* If 1, map \r to \n on input	  */
				/* and map both to \n\r on output */
void echo	() {  Echo = 1; }
void noecho	() {  Echo = 0; }
void nl		() {  Nl   = 1; }
void nonl	() {  Nl   = 0; }

void crmode()
{
      /* setvbuf(stdin, NULL, _IONBF, 0);  /* Turn off buffering */
    Crmode = 1;
}

void nocrmode()
{
     /* freopen( "/dev/con", "r", stdin ); */
    Crmode = 0;
}

/*--------------------------------------------------------*/

static  char	*getbuf(win, buf)
WINDOW	*win;
char	*buf;
{
    /* Get a buffer interactively. ^H is a destructive backspace. This routine
     * is mildly recursive (it's called from wgetch() when Crmode is false.
     * The newline is put into the buffer. Returns it's second argument.
     */

    int  c;
    char *sbuf = buf;

    Crmode = 1;
    while( (c = wgetch(win)) != '\n' && c != '\r' )
    {
	switch( c )
	{
	case '\b':  if( buf > sbuf )
	            {
			--buf;
			wprintw( win, " \b" );
		    }
		    else
		    {
			wprintw( win, " " );
			putchar('\007');
		    }
		    break;

	default:    *buf++ = c;
		    break;
	}
    }
    *buf++ = c   ;	/* Add line terminator (\n or \r) */
    *buf   = '\0';
    Crmode = 0   ;
    return sbuf  ;
}

/*--------------------------------------------------------*/

int	wgetch( win )
WINDOW	*win;
{
    /* Get a character from DOS without echoing. We need to do this in order
     * to support (echo/noecho). We'll also do noncrmode input buffering here.
     * Maximum input line length is 132 columns.
     *
     * In nocrmode(), DOS functions are used to get a line and all the normal
     * command-line editing functions are available. Since there's no way to
     * turn off echo in this case, characters are echoed to the screen
     * regardless of the status of echo(). In order to retain control of the
     * window, input fetched for wgetch() is always done in crmode, even if
     * Crmode isn't set. If nl() mode is enabled, carriage return (Enter, ^M)
     * and linefeed (^J) are both mapped to '\n', otherwise they are not mapped.
     *
     * Characters are returned in an int. The high byte is 0 for normal
     * characters. Extended characters (like the function keys) are returned
     * with the high byte set to 0xff and the character code in the low byte.
     * Extended characters are not echoed.
     */

    static unsigned char  buf[ 133 ];	/* Assuming initialization to 0's */
    static unsigned char  *p = buf;
    register	    int	  c;

    if( !Crmode )
    {
	if( !*p )
	    p = getbuf( win, buf );
	return( Nl && *p == '\r' ) ? '\n' : *p++ ;
    }
    else
    {
	if( (c = bdos(8,0,0) & 0xff) == ('Z'-'@') )	/* Ctrl-Z */
		return EOF ;
	else if( !c )					/* Extended char */
		c = ~0xff | bdos(8,0,0) ;
	else
	{
	    if( c == '\r' &&  Nl )
		c = '\n' ;

	    if( Echo )
		waddch( win, c );
	}
	return c;
    }
}

/*--------------------------------------------------------*/

int	waddch( win, c )
WINDOW	*win;
int	c;
{
    /*	Print a character to an active (not hidden) window:
     *	The following are handled specially:
     *
     *	\n  Clear the line from the current cursor position  to the right edge
     *	    of the window. Then:
     *	        if nl() is active:
     *		    go to the left edge of the next line
     *	        else
     *		    go to the current column on the next line
     *	    In addition, if scrolling is enabled, the window scrolls if you're
     *	    on the bottom line.
     *	\t  is expanded into an 8-space field. If the tab goes past the right
     *      edge of the window, the cursor wraps to the next line.
     *	\r  gets you to the beginning of the current line.
     *	\b  backs up one space but may not back up past the left edge of the
     *      window. Nondestructive. The curses documentation doesn't say that
     *      \b is handled explicitly but it does indeed work.
     *	ESC This is not standard curses but is useful. It is valid only if
     *      DOESC was true during the compile. All characters between an ASCII
     *	    ESC and an alphabetic character are sent to the output but are other
     *      wise ignored. This let's you send escape sequences directly to the
     *      terminal if you like. I'm assuming here that you won't change
     *      windows in the middle of an escape sequence.
     *
     * Return ERR if the character would have caused the window to scroll
     * illegally, or if you attempt to write to a hidden window.
     */

#ifdef DOESC
    static  int	    saw_esc = 0;
#endif
    int		    rval    = OK;

    if( win->hidden )
	return ERR;

    cmove(win->y_org + win->row, win->x_org + win->col);

#ifdef DOESC
    if( saw_esc )
    {
	if( isalpha( c ) )
	    saw_esc = 0;
	outc(c, win->attrib );
    }
    else
#endif
    {
	switch( c )
	{
#ifdef DOESC
	case '\033': if( saw_esc )
			saw_esc = 0;
		     else
		     {
			saw_esc = 1;
			outc('\033', win->attrib );
		     }
		     break;
#endif
	case '\b':  if( win->col > 0 )
		    {
		        outc( '\b', win->attrib  );
		        --( win->col );
		    }
		    break;

	case '\t':  do {
			waddch( win, ' ' );
		    } while( win->col % 8 );
		    break;

	case '\r':  win->col = 0;
		    cmove(win->y_org + win->row, win->x_org);
		    break;

	default:    if( (win->col + 1) < win->x_size )
		    {
			/* If you're not at the right edge of the window, */
			/* print the character and advance.		  */

			++win->col;
			outc(c, win->attrib );
			break;
		    }
		    replace(c);	  	  /* At right edge, don't advance */
		    if( !win->wrap_ok )
			break;

		    /* otherwise wrap around by falling through to newline */

	case '\n':  if( c == '\n' )	   /* Don't erase character at far */
			wclrtoeol( win );  /* right of the screen.	   */

		    if( Nl )
			win->col = 0;

		    if( ++(win->row) >= win->y_size )
		    {
			rval = wscroll( win, 1 );
			--( win->row );
		    }
		    cmove( win->y_org + win->row, win->x_org + win->col);
		    break;
	}
    }
    return rval;
}
