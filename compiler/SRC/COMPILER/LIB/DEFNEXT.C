/*@A (C) 1992 Allen I. Holub                                                */

#include <stdio.h>
#include <tools/debug.h>
#include <tools/compiler.h>	/* needed only for prototype */

PUBLIC	void	defnext(fp, name)
FILE	*fp;
char	*name;
{
    /* Print the default yy_next(s,c) subroutine for an uncompressed table. */

    static char	 *comment_text[] =
    {
	"yy_next(state,c) is given the current state and input character and",
	"evaluates to the next state.",
	NULL
    };

    comment( fp, comment_text );
    fprintf( fp, "#define yy_next(state, c)  %s[ state ][ c ]\n", name );
}

