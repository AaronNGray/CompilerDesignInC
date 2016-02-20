/*@A (C) 1992 Allen I. Holub                                                */
#include "video.h"
#include <tools/termlib.h>

void cpy_row ( int dest_row, int src_row, int left_col, int right_col	);
void cpy_col ( int dest_col, int src_col, int top_row,  int bot_row	);
void clr_col ( int col, int attrib, int top_row,  int bot_row   	);
void clr_row ( int row, int attrib, int left_col, int right_col		);

static	void cpy_row( dest_row, src_row, left_col, right_col )
int dest_row, src_row, left_col, right_col;
{
    /* Copy all characters between left_col and right_col (inclusive)
     * from src_row to the equivalent position in dest_row.
     */

    CHARACTER FARPTR s ;
    CHARACTER FARPTR d ;

    d = & SCREEN[ dest_row ][ left_col ];
    s = & SCREEN[ src_row  ][ left_col ];

    while( left_col++ <= right_col )
	*d++ = *s++;
}

/*----------------------------------------------------------------------*/

static	void cpy_col( dest_col, src_col, top_row, bot_row )
int dest_col, src_col, top_row, bot_row ;
{
    /* Copy all characters between top_row and bot_row (inclusive)
     * from src_col to the equivalent position in dest_col.
     */

    CHARACTER FARPTR s = & SCREEN[ top_row ][ src_col  ];
    CHARACTER FARPTR d = & SCREEN[ top_row ][ dest_col ];

    while( top_row++ <= bot_row )
    {
	*d = *s;
	d += NUMCOLS;
	s += NUMCOLS;
    }
}

/*----------------------------------------------------------------------*/

static	void clr_row( row, attrib, left_col, right_col )
int row, attrib, left_col, right_col ;
{
    /* Clear all characters in the indicated row that are between left_col and
     * right_col (inclusive).
     */

    CHARACTER FARPTR p = & SCREEN[ row ][ left_col ];

    while( left_col++ <= right_col )
    {
	(p  )->letter    = ' ';
	(p++)->attribute = attrib ;
    }
}

/*----------------------------------------------------------------------*/

static	void clr_col( col, attrib, top_row, bot_row )
int col, attrib, top_row, bot_row ;
{
    /* Clear all characters in the indicated column that are between top_row
     * and bot_row (inclusive).
     */

    CHARACTER FARPTR p = & SCREEN[ top_row ][ col ];

    while( top_row++ <= bot_row )
    {
	    p->letter    = ' ';
	    p->attribute = attrib ;
	    p += NUMCOLS ;
    }
}

/*======================================================================*
 *            	     Externally accessible functions:			*
 *======================================================================*
 */

void dv_scroll_line ( x_left, x_right, y_top, y_bottom, dir, attrib )
int x_left, x_right, y_top, y_bottom, dir, attrib;
{
    /* Scroll the window located at:
     *
     * (y_top, x_left) 	+---------+
     *			|	  |
     *			|	  |
     *			+---------+ (y_bottom, x_right)
     *
     * Dir is one of:  'u', 'd', 'l', or 'r' for up, down, left, or right.
     * The cursor is not moved. The opened line is filled with space characters
     * having the indicated attribute.
     */

    int  i;

    if( dir == 'u' )
    {
	for( i = y_top; i < y_bottom ; i++ )
	    cpy_row( i, i+1, x_left, x_right );
	clr_row( y_bottom, attrib, x_left, x_right );
    }
    else if( dir == 'd' )
    {
	for( i = y_bottom; --i >= y_top ; )
	    cpy_row( i+1, i, x_left, x_right );
	clr_row( y_top, attrib, x_left, x_right );
    }
    else if( dir == 'l' )
    {
	for( i = x_left; i < x_right; i++ )
	    cpy_col( i, i+1, y_top, y_bottom );
	clr_col( x_right, attrib, y_top, y_bottom );
    }
    else /* dir == 'r' */
    {
	for( i = x_right; --i >= x_left ; )
	    cpy_col( i+1, i, y_top, y_bottom );
	clr_col( x_left, attrib, y_top, y_bottom );
    }
}

/*----------------------------------------------------------------------*/

void dv_scroll( x_left, x_right, y_top, y_bottom, amt, attrib )
int x_left, x_right, y_top, y_bottom, amt, attrib;
{
    /* Scroll the screen up or down by the indicated amount. Negative
     * amounts scroll down.
     */

    int dir = 'u';

    if( amt < 0 )
    {
	amt = -amt;
	dir = 'd' ;
    }
    while( --amt >= 0 )
	dv_scroll_line( x_left, x_right, y_top, y_bottom, dir, attrib );
}
