/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/set.h>
#include "parser.h"
#include "../version.h"

#if ( 0 UNIX(+1) )
#include "date.h"	/* Otherwise use ANSI __DATE__ */
#endif

PUBLIC void signon()
{
    /* Print the sign-on message. Note that since the console is opened
     * explicitly, the message is printed even if both stdout and stderr are
     * redirected.
     */

    FILE *screen;
    if( !(screen = fopen( UNIX("/dev/tty") MS("con:"), "w")) )
	  screen = stderr;

    fprintf(screen,"%s %s (c) %s, Allen I. Holub. All rights reserved.\n",
				LL("LLAMA") OX("OCCS"), VERSION, __DATE__);
    if( screen != stderr )
	fclose(screen);
}
