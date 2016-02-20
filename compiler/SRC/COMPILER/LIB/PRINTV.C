/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>

void	printv( fp, argv )
FILE	*fp;
char	**argv;
{
    /* Print an argv-like array of pointers to strings, one string per line.
     * The array must be NULL terminated.
     */
    while( *argv )
	fprintf( fp, "%s\n", *argv++ );
}

void	comment( fp, argv )
FILE	*fp;
char	**argv;
{
    /* Works like printv except that the array is printed as a C comment. */

    fprintf(fp, "\n/*-----------------------------------------------------\n");
    while( *argv )
	    fprintf(fp, " * %s\n",  *argv++ );
    fprintf(fp, " */\n\n");
}
