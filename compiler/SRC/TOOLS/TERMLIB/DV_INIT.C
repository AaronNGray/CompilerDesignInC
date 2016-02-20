/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/termlib.h>
#include "video.h"
int	dv_init()
{
    int i;
    if( (i = vb_iscolor()) >= 0 )
	dv_Screen = i ? COLBASE : MONBASE ;
    return( i != -1 );
}
