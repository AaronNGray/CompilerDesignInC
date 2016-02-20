/*@A (C) 1992 Allen I. Holub                                                */
#include "video.h"

void	dv_clrs( attrib )
{
    /*  Clear the screen. The cursor is not moved.  */

    CHARACTER FARPTR p = (CHARACTER FARPTR)( dv_Screen );
    int     i ;

    for( i = NUMROWS * NUMCOLS;  --i >= 0 ; )
    {
	(p  )->letter    = ' ';
	(p++)->attribute = attrib ;
    }
}
