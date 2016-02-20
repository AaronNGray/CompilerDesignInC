/*@A (C) 1992 Allen I. Holub                                                */
#include "video.h"
#include <tools/termlib.h>

void	dv_putchar( c )
int c;
{
    dv_putc( c & 0xff, NORMAL );
}
