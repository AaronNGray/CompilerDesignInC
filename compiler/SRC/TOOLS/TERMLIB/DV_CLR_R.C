/*@A (C) 1992 Allen I. Holub                                                */
#include "video.h"

void	dv_clr_region( l, r, t, b, attrib )
{
    int	  ysize, xsize, x, y ;

    xsize = (r - l) + 1;		/* horizontal size of region */
    ysize = (b - t) + 1;		/* vertical size of region   */

    for( y = ysize; --y >= 0 ;)
	for( x = xsize; --x >= 0 ; )
	{
    	    SCREEN[ y + t ][ x + l ].letter    = ' ';
    	    SCREEN[ y + t ][ x + l ].attribute = attrib ;
	}
}
