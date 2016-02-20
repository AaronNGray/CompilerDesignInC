/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <ctype.h>
#include <tools/debug.h>
#include "globals.h"
#include "dfa.h"	/* lerror() prototype */

/* INPUT.C	Lowest-level input functions.  */

static  int getline  P((char **stringp, int n, FILE *stream));
/*----------------------------------------------------------------------*/
PUBLIC char	*get_expr( ANSI(void) )
{
    /* Input routine for nfa(). Gets a regular expression and the associated
     * string from the input stream. Returns a pointer to the input string
     * normally. Returns NULL on end of file or if a line beginning with % is
     * encountered. All blank lines are discarded and all lines that start with
     * whitespace are concatenated to the previous line. The global variable
     * Lineno is set to the line number of the top line of a multiple-line
     * block. Actual_lineno holds the real line number.
     */

    static  int	lookahead = 0;
    int		space_left;
    char	*p;

    p 	       = Input_buf;
    space_left = MAXINP;
    if( Verbose > 1 )
	printf( "b%d: ", Actual_lineno );

    if( lookahead == '%' )	/* next line starts with a % sign	*/
	return NULL;	    	/* return End-of-input marker		*/

    Lineno = Actual_lineno ;

    while( (lookahead = getline(&p, space_left-1, Ifile)) != EOF )
    {
	if( lookahead == 0 )
	    lerror(1, "Rule too long\n");

	Actual_lineno++;

	if( !Input_buf[0] )			/* Ignore blank lines */
	    continue;

	space_left = MAXINP - (p-Input_buf);

	if( !isspace(lookahead) )
	    break;

	*p++ = '\n' ;
    }

    if( Verbose > 1 )
	printf( "%s\n", lookahead ? Input_buf : "--EOF--" );

    return lookahead ? Input_buf : NULL ;
}

/*----------------------------------------------------------------------*/

PRIVATE int getline( stringp, n, stream )
char	**stringp;
int	n;
FILE	*stream;
{
    /* Gets a line of input. Gets at most n-1 characters. Updates *stringp
     * to point at the '\0' at the end of the string. Return a lookahead
     * character (the character that follows the \n in the input). The '\n'
     * is not put into the string.
     *
     * Return the character following the \n normally,
     *        EOF  at end of file,
     *	      0    if the line is too long.
     */

    static int	lookahead  = 0;
    char	*str	   = *stringp;

    if( lookahead == 0 )			/* initialize */
	lookahead = getc( stream );

    if( n > 0  && lookahead != EOF )
    {
	while( --n > 0 )
	{
	    *str      = lookahead ;
	    lookahead = getc(stream);

	    if( *str == '\n' || *str == EOF )
		break;
	    ++str;
	}
	*str     = '\0';
	*stringp = str ;
    }
    return (n <= 0) ? 0 : lookahead ;
}
