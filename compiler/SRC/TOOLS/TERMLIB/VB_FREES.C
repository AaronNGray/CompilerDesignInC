/*@A (C) 1992 Allen I. Holub                                                */
#include <stdlib.h>
#include <tools/termlib.h>
#include "video.h"

void	vb_freesbuf( p )
SBUF	*p;
{				/* Free an SBUF as is allocated by vb_save(). */
    IFREE( p->image );
    free ( p );
}
