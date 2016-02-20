/*@A (C) 1992 Allen I. Holub                                                */

#include <stdio.h>
#include <tools/debug.h>
#include <tools/compiler.h>		/* for prototypes only */

/* PAIRS.C	This module contains the routines to compress a table
 *		horizontally (using char/next pairs) and then print the
 *		compressed table. The compressed array looks like this:
 *
 *  Yy_nxt:      Yy_nxtDD:
 *  +------+	 +-------+----------------------------------------+
 *  |   *--|---->|   0   | Next state array, indexed by character |
 *  +------+	 +-------+----------------------------------------+
 *  |      |
 *  +------+	 +-------+----+----+----+----+-----+
 *  |   *--|---->| count | c1 | s1 | c2 | s2 | ... |
 *  +------+	 +-------+----+----+----+----+-----+
 *  | NULL |  			(there are no non-failure transitions if NULL)
 *  +------+
 */

typedef int	 ATYPE;	     /* Declared type of input tables		*/


#define NCOLS	 10   	     /* Number of columns used to print arrays	*/
#define TYPE	 "YY_TTYPE"  /* Declared type of output tables.		*/
#define SCLASS   "YYPRIVATE" /* Storage class of all the tables		*/
#define D_SCLASS "YYPRIVATE" /* Storage class of the decoding routine	*/
/*----------------------------------------------------------------------*/

PUBLIC	int  pairs( fp, array, nrows, ncols, name, threshold, numbers )

FILE    *fp;	   		/* output file				*/
ATYPE	*array;			/* DFA transition table		 	*/
int	nrows;			/* Number of rows in array[]	 	*/
int	ncols;			/* Number of columns in array[]	 	*/
char	*name;			/* Name used for output array.		*/
int	threshold;		/* Array vs. pairs threshold	 	*/
int	numbers;		/* Use numbers for char. part of pair	*/
{
    /* Generate the C source code for a pair-compressed DTRAN. Returns the
     * number of cells used for the YysDD arrays. The "numbers" argument
     * determines the output format of the character part of a
     * character/next-state pair. If numbers is true, then normal numbers
     * are used, otherwise ASCII characters are used. For example: 'a',100
     * as compared to: 97,100 ('a' == 0x61 == 97, decimal).
     */

    int	    i, j, ntransitions, nprinted, ncommas;
    int	    num_cells = 0;		   	       /* cells used for rows */
    ATYPE   *p;
    char    *bin_to_ascii();

    for( i = 0; i < nrows ; i++ )
    {
	ntransitions = 0;
	for( p = array + (i * ncols), j = ncols; --j >= 0; p++ )
	    if( *p != -1 )
		++ntransitions ;

	if( ntransitions > 0 )
	{
	    fprintf(fp, "%s %s %s%-2d[] = { ", SCLASS, TYPE, name, i );    /*}*/
	    ++num_cells;
	    if( ntransitions > threshold )			    /* array */
		fprintf(fp, "0,\n               ");
	    else						    /* pairs */
	    {
		fprintf(fp, "%2d, ", ntransitions );
		if( threshold > 5 )
		    fprintf(fp, "\n               " );
	    }

	    nprinted = NCOLS;
	    ncommas  = ntransitions;

	    for( p = array + (i * ncols),  j = 0;  j < ncols; j++, p++ )
	    {
		if( ntransitions > threshold )			    /* array */
		{
		    ++num_cells ;
		    --nprinted  ;

		    fprintf(fp, "%3d" , *p );
		    if( j < ncols-1 )
			fprintf(fp, ", ");
		}
		else if( *p != -1 )				    /* pairs */
		{
		    num_cells += 2;

		    if( numbers )
		    	fprintf(fp, "%d,%d", j, *p );
		    else
			fprintf(fp, "'%s',%d", bin_to_ascii(j,0), *p );

		    nprinted -= 2;
		    if( --ncommas > 0 )
			fprintf(fp, ", ");
		}

		if( nprinted <= 0 )
		{
		    fprintf(fp, "\n               ");
		    nprinted = NCOLS;
		}
	    }
	    fprintf(fp, "};\n" );
	}
    }

    fprintf(fp,"\n%s %s *%s[ %d ] =\n{\n    ", SCLASS, TYPE, name, nrows );

    nprinted = 10;				/* 10 elements on a line */
    for( --nrows, i = 0; i < nrows ; i++ )
    {
	ntransitions = 0;
	for( p = array + (i * ncols), j = ncols; --j >= 0; p++ )
	    if( *p != -1 )
		++ntransitions ;

	fprintf(fp, ntransitions ? "%s%-2d, " : "NULL,  ", name, i );

	if( --nprinted <= 0 )
	{
	    fprintf(fp,"\n    ");
	    nprinted = 10;
	}
    }

    fprintf(fp, "%s%-2d\n};\n\n", name, i );
    return num_cells;
}

/*----------------------------------------------------------------------*/

PUBLIC	void	pnext( fp, name )
char	*name;
FILE	*fp;
{
    /*  Print out a next(state,c) subroutine for a table compressed	*/
    /*  into char/next-state pairs by the routines in pairs.c.		*/

    static char	*toptext[]=
    {
      "unsigned int   c ;",
      "int            cur_state ;",
      "{",
      "    /* Given the current state and the current input character, return",
      "     * the next state.",
      "     */",
      "",
      NULL
    };
    static char	*boptext[]=
    {
      "    int i;",
      "",
      "    if( p )",
      "    {",
      "        if( (i = *p++)  == 0 )",
      "            return  p[ c ];",
      "",
      "        for( ; --i >= 0 ; p += 2 )",
      "            if( c == p[0] )",
      "                 return p[1];",
      "    }",
      "    return  YYF;",
      "}",
      NULL
    };
    fprintf (fp, "\n/*--------------------------------------------------*/\n");
    fprintf (fp, "%s %s yy_next( cur_state, c )\n", D_SCLASS, TYPE	     );
    printv  (fp, toptext );
    fprintf (fp, "    %s   *p = %s[ cur_state ] ;\n", TYPE, name );
    printv  (fp, boptext );
}
