/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <tools/debug.h>
#include <tools/l.h>
#include <tools/occs.h>
#include "proto.h"

void	main( argc, argv )
int	argc;
char	**argv;
{
     /*  malloc_checking(1,1); */

    argc = yy_get_args( argc, argv );

    if( argc > 2 )	   /* Generate trace if anything follows file name on */
	enable_trace();	   /* command line.				      */

#ifdef YACC
    UNIX( yy_init_occs(); ) /* Needed for yacc, called automatically by occs. */
#endif
    yyparse();
}

/*----------------------------------------------------------------------*/
void yyhook_a()		/* Print symbol table from debugger with ctrl-A. */
{			/* Not used in yacc-generated parser.		 */
    static int	x = 0;
    char	buf[32];

    sprintf    ( buf, "sym.%d", x++ );
    yycomment  ( "Writing symbol table to %s\n", buf );
    print_syms ( buf );
}

void yyhook_b()		/* Enable/disable run-time trace with Ctrl-b.       */
{			/* Not used in yacc-generated parser.		    */
			/* enable_trace() and disable_trace() are discussed */
			/* below, when the gen() call is discussed.	    */
    char buf[32];
    if( yyprompt( "Enable or disable trace? (e/d): ", buf, 0 ) )
    {
	if( *buf == 'e' ) enable_trace();
	else		  disable_trace();
    }
}
