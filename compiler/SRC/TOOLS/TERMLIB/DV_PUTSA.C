/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/termlib.h>
#include "video.h"

void	dv_putsa( str, attrib )
char	*str;
int	attrib;
{
    /* Write string to screen, giving characters the indicated attributes. */
    while( *str )
	dv_putc( *str++, attrib );
}
