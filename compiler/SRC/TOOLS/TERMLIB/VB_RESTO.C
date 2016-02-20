/*@A (C) 1992 Allen I. Holub                                                */
#include <stdlib.h>
#include <tools/vbios.h>
#include "video.h"

SBUF	*vb_restore( sbuf )
SBUF	*sbuf;
{
    /* Restore a region saved with a previous vb_save() call. The cursor is
     * not modified. Note that the memory used by sbuf is not freed, you must
     * do that yourself with a vb_freesbuf(sbuf) call.
     */

    int	    ysize, xsize, x, y ;
    IMAGEP  p;

    xsize = ( sbuf->right  - sbuf->left ) + 1 ;
    ysize = ( sbuf->bottom - sbuf->top  ) + 1 ;
    p     = sbuf->image;

    for( y = 0; y < ysize ; ++y )
	for( x = 0; x < xsize ; ++x )
	{
    	    VB_CTOYX( y + sbuf->top, x + sbuf->left );
	    VB_OUTCHA( *p );
	    ++p;	      /* VB_OUTCHA has side effects so can't use *p++ */
	}
    return sbuf;
}
