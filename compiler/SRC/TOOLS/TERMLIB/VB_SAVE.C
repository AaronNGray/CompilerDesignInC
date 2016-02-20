/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <tools/termlib.h>  /* must be included first */
#include <tools/vbios.h>

SBUF *vb_save( l, r, t, b )
int l, r, t, b;
{
    /* Save all characters and attributes in indicated region. Return a pointer
     * to a save buffer. The cursor is not modified. Note that the save buffer
     * is allocated from the far heap but the sbuf itself is not. See also,
     * dv_restore() and dv_freesbuf();
     */

    int		ysize, xsize, x, y ;
    IMAGEP	p;
    SBUF	*sbuf;

    xsize = (r - l) + 1;
    ysize = (b - t) + 1;

    if( !(sbuf = (SBUF *) malloc(sizeof(SBUF)) ))
    {
	fprintf(stderr, "Internal error (vb_save): No memory for SBUF.");
	exit( 1 );
    }

    if( !(p = (IMAGEP) IMALLOC(xsize * ysize * sizeof(WORD))))
    {
	fprintf(stderr, "Internal error (vb_save): No memory for image.");
	exit( 2 );
    }

    sbuf->left   = l;
    sbuf->right  = r;
    sbuf->top    = t;
    sbuf->bottom = b;
    sbuf->image  = p;

    for( y = 0; y < ysize ; ++y )
	for( x = 0; x < xsize ; ++x )
	{
	    VB_CTOYX( y + t , x + l );
    	    *p++ = VB_INCHA();
	}

    return sbuf;
}
