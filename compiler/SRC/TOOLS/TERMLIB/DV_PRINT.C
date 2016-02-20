/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdarg.h>
#include <curses.h>		/* For prnt() prototype	  */
#include <tools/debug.h>	/* For VA_LIST definition */
#include <tools/termlib.h>	/* For dv_putc definition */
#include "video.h"

void	dv_printf( attrib, fmt  VA_LIST )
int	attrib;
char	*fmt;
{
    /* Direct-video printf, characters will have the indicated attributes.  */
    /* prnt() is in curses.lib, which must be linked to your program if you */
    /* use the current function.					    */

    va_list args;

    va_start( args, fmt );
    prnt    ( dv_putc, attrib, fmt, args );
    va_end  ( args );
}
