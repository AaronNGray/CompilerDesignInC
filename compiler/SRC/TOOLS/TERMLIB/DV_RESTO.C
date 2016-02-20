/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/termlib.h>
#include "video.h"

SBUF	*dv_restore( sbuf )
SBUF	*sbuf;
{
    /* Restore a region saved with a previous dv_save() call. The cursor is
     * not modified. Note that the memory used by sbuf is not freed, you must
     * do that yourself with a dv_freesbuf(sbuf) call.
     */

    int	    ysize, xsize, x, y ;
    IMAGEP  p;

    xsize = ( sbuf->right  - sbuf->left ) + 1 ;
    ysize = ( sbuf->bottom - sbuf->top  ) + 1 ;
    p     = sbuf->image;

    for( y = 0; y < ysize ; ++y )
	for( x = 0; x < xsize ; ++x )
    	    VSCREEN[ y + sbuf->top ][ x + sbuf->left ] = *p++;

    return sbuf;
}
