/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include "../version.h"

#if (0  KnR(+1))
#include "date.h" /* #define for __DATE__, created from makefile */
#endif

void signon( ANSI(void) )
{
    /* Print the sign-on message. Since the console is opened explicitly, the
     * message is printed even if both stdout and stderr are redirected.
     */

    FILE *screen;

    UNIX( if( !(screen = fopen("/dev/tty", "w")) )	)
    MS  ( if( !(screen = fopen("con:",     "w")) )	)
	    screen = stderr;

    /* The ANSI __DATE__ macro yields a string of the form: "Sep 01 1989".
     * The __DATE__+7 gets the year portion of that string.
     *
     * The UNIX __DATE__ is created from the makefile with this command:
     *		echo \#define __DATE__ \"`date`\" >> date.h
     * which evaluates to:
     *		#define __DATE__ "Sun Apr 29 12:51:50 PDT 1990"
     * The year is at __DATE__+24
     */

    fprintf(screen,"LeX %s [%s]. (c) %s, Allen I. Holub.", VERSION,
    						__DATE__,
						ANSI( __DATE__+7 )
						KnR ( __DATE__+24) );
    fprintf(screen," All rights reserved.\n");

    if( screen != stderr )
	fclose(screen);
}
