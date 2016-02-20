/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/compiler.h>
#include "dfa.h"
#include "nfa.h"
#include "globals.h"

/*  PRINT.C:  This module contains miscellaneous print routines that do
 *  everything except print the actual tables.
 */

PUBLIC void pheader P(( FILE *fp,     ROW dtran[], int nrows, ACCEPT *accept ));
PUBLIC void pdriver P(( FILE *output,              int nrows, ACCEPT *accept ));
/*------------------------------------------------------------*/

PUBLIC void pheader( fp, dtran, nrows, accept )
FILE	*fp;			/* output stream			*/
ROW	dtran[];		/* DFA transition table			*/
int	nrows;			/* Number of states in dtran[]		*/
ACCEPT	*accept;		/* Set of accept states in dtran[]	*/
{
    /*  Print out a header comment that describes the uncompressed DFA. */

    int		i, j;
    int		last_transition ;
    int		chars_printed;
    char	*bin_to_ascii() ;

    fprintf(fp, "#ifdef __NEVER__\n" );
    fprintf(fp, "/*---------------------------------------------------\n");
    fprintf(fp, " * DFA (start state is 0) is:\n *\n" );

    for( i = 0; i < nrows ; i++ )
    {
	if( !accept[i].string )
	    fprintf(fp, " * State %d [nonaccepting]", i );
	else
	{
	    fprintf(fp, " * State %d [accepting, line %d <",
				    i , ((int *)(accept[i].string))[-1] );

	    fputstr( accept[i].string, 20, fp );
	    fprintf(fp, ">]" );

	    if( accept[i].anchor )
		fprintf( fp, " Anchor: %s%s",
				    accept[i].anchor & START ? "start " : "",
				    accept[i].anchor & END   ? "end"    : "" );
	}

	last_transition = -1;
	for( j = 0; j < MAX_CHARS; j++ )
	{
	    if( dtran[i][j] != F )
	    {
		if( dtran[i][j] != last_transition )
		{
		    fprintf(fp, "\n *    goto %2d on ", dtran[i][j] );
		    chars_printed = 0;
		}

		fprintf(fp, "%s", bin_to_ascii(j,1) );

		if( (chars_printed += strlen(bin_to_ascii(j,1))) > 56 )
		{
		    fprintf(fp, "\n *               " );
		    chars_printed = 0;
		}

		last_transition = dtran[i][j];
	    }
	}
	fprintf(fp, "\n");
    }
    fprintf(fp," */\n\n"  );
    fprintf(fp,"#endif\n" );
}

/*--------------------------------------------------------------*/

PUBLIC	void	pdriver( output, nrows, accept )
FILE	*output;
int	nrows;		/* Number of states in dtran[]		*/
ACCEPT	*accept;	/* Set of accept states in dtran[]	*/
{
    /* Print the array of accepting states, the driver itself, and the case
     * statements for the accepting strings.
     */

    int	    i;
    static  char  *text[] =
    {
    	"The Yyaccept array has two purposes. If Yyaccept[i] is 0 then state",
    	"i is nonaccepting. If it's nonzero then the number determines whether",
    	"the string is anchored, 1=anchored at start of line, 2=at end of",
	"line, 3=both, 4=line not anchored",
	NULL
    };

    comment( output, text );
    fprintf(output, "YYPRIVATE YY_TTYPE  Yyaccept[] =\n" );
    fprintf(output, "{\n"			         );

    for( i = 0 ; i < nrows ; i++ )			/* accepting array */
    {
	if( !accept[i].string )
	    fprintf( output, "\t0  " );
	else
	    fprintf( output, "\t%-3d",accept[i].anchor ? accept[i].anchor :4);

	fprintf(output, "%c    /* State %-3d */\n",
				i == (nrows -1) ? ' ' : ',' , i );
    }
    fprintf(output, "};\n\n" );

    driver_2( output, !No_lines );			/* code above cases */

    for( i = 0 ; i < nrows ; i++ )			/* case statements  */
    {
	if( accept[i].string )
	{
	    fprintf(output, "\t\tcase %d:\t\t\t\t\t/* State %-3d */\n",i,i);
	    if( !No_lines )
		fprintf(output, "#line %d \"%s\"\n",
					    *( (int *)(accept[i].string) - 1),
					    Input_file_name );

	    fprintf(output, "\t\t    %s\n",    accept[i].string	 );
	    fprintf(output, "\t\t    break;\n" 		 );
	}
    }

    driver_2( output, !No_lines );		/* code below cases */
}
