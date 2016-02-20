/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>		/* FILE def needed by l.h.		*/
#include <stdlib.h>		/* includes extern definition for errno */
#include <stdarg.h>		/* va_list def needed by l.h.		*/
#include <tools/debug.h>
#include <tools/l.h>		/* Needed only for prototypes		*/

/* This is the default routine called by ferr when it exits. It should return
 * the exit status. You can supply your own version of this routine if you like.
 */
int	on_ferr()
{
    UNIX( extern int errno; )
    return errno;
}
