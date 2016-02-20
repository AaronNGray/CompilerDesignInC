/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/vbios.h>

void	vb_getyx( yp, xp )
int	*yp, *xp;
{
    int	posn;

    posn = VB_GETCUR();
    *xp  = posn	   & 0xff ;
    *yp  = (posn >> 8) & 0xff ;
}
