/*@A (C) 1992 Allen I. Holub                                                */
#include <stdlib.h>
#include <tools/termlib.h>
#include "video.h"
				/* Free an SBUF as is allocated by vb_save(). */
void	dv_freesbuf( p )
SBUF	*p;
{
    IFREE( p->image );
    free ( p );
}
