/*@A (C) 1992 Allen I. Holub                                                */
#include "video.h"
#include <tools/vbios.h>

static int	Row = 0;	/* Cursor Row	 */
static int	Col = 0;	/* Cursor Column */

/*----------------------------------------------------------------------*/
/* Move cursor to (Row, Col) 						*/

#define fix_cur() _Vbios( SET_POSN, 0, 0, 0, (Row << 8) | Col , "ax" )

/*----------------------------------------------------------------------*/

void	dv_putc( c, attrib )
int c;
int attrib;
{
    /* Write a single character to the screen with the indicated  attribute.
     * The following are special:
     *
     *	\0	ignored
     *	\f	clear screen and home cursor
     *	\n	to left edge of next line
     *	\r	to left edge of current line
     *	\b	one character to left (non-destructive)
     *
     * The screen will scroll up if you go past the bottom line. Characters
     * that go beyond the end of the current line wrap around to the next line.
     */

    switch( c )
    {
    case 0:	break;				/* Ignore ASCII NULL's */

    case '\f':	dv_clrs( attrib );
		    Row = Col = 0;
		    break;

    case '\n':	if( ++Row >= NUMROWS )
		{
		    dv_scroll_line(0,79,0,24, 'u', NORMAL );
		    Row = NUMROWS-1 ;
		}
						/* Fall through to '\r' */
    case '\r':	Col = 0;
		break;

    case '\b':	if( --Col < 0 )
		    Col = 0;
		break;

    default :   SCREEN[ Row ][ Col ].letter    = c      ;
		SCREEN[ Row ][ Col ].attribute = attrib ;
		if( ++Col >= NUMCOLS )
		{
		    Col = 0;
		    if( ++Row >= NUMROWS )
		    {
			dv_scroll_line(0,79,0,24, 'u', NORMAL );
			Row = NUMROWS-1 ;
		    }
		}
		break;
    }
    fix_cur();
}

/*----------------------------------------------------------------------*/

void dv_ctoyx( y, x )
int y, x;
{ 		       /* Position the cursor at the indicated row and column */
    Row = y;
    Col = x;
    fix_cur();
}

/*----------------------------------------------------------------------*/

void	dv_getyx( rowp, colp )
int	*rowp, *colp;
{ 	               /* Modify *rowp and *colp to hold the cursor position. */
    *rowp = Row;
    *colp = Col;
}

/*----------------------------------------------------------------------*/

int	dv_incha() 		/* Get character & attribute from screen. */
{
    return  (int)( VSCREEN[ Row ][ Col ] );
}

void dv_outcha(c)	 	/* Write char. & attrib. w/o moving cursor. */
int c;
{
    VSCREEN[ Row ][ Col ] = c ;
}

void  dv_replace(c)		/* Write char. only w/o moving cursor */
int c;
{
    SCREEN[ Row ][ Col ].letter = c ;
}
