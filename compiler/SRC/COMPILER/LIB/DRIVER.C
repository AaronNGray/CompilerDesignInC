/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/compiler.h>	/* for prototypes */

/*------------------------------------------------------------*/

PUBLIC FILE *driver_1  P((  FILE *output, int line,  char *file_name	));
PUBLIC int   driver_2  P((  FILE *output, int line			));

PRIVATE FILE *Input_file = NULL ;
PRIVATE int  Input_line;	   /* line number of most-recently read line */
PRIVATE char File_name[80];	   /* template-file name		     */

/*------------------------------------------------------------*/

PUBLIC FILE *driver_1( output, lines, file_name )
FILE	*output;
int	lines;
char	*file_name;
{
    char path[80];
    UNIX( extern int errno; )

    if( !(Input_file = fopen( file_name, "r" )) )
    {
	searchenv( file_name, "LIB", path );
	if( !*path )
	{
	    errno = ENOENT;	/* Can't find file, simulate ENOENT */
	    return NULL;	/* (file not found) error.          */
	}
	if( !(Input_file = fopen( path, "r")) )
	    return NULL;
    }

    strncpy( File_name, file_name, sizeof(File_name) );
    Input_line = 0;
    driver_2( output, lines );
    return Input_file;
}

/*--------------------------------------------------------------*/

PUBLIC int driver_2( output, lines )
FILE	*output;
int	lines;
{
    static char buf[ 256 ];
    char	*p;
    int		processing_comment = 0;

    if( !Input_file )
	ferr( "INTERNAL ERROR [driver_2], Template file not open.\n");

    if( lines )
	fprintf( output, "\n#line %d \"%s\"\n", Input_line + 1, File_name );

    while( fgets(buf, sizeof(buf), Input_file) )
    {
	++Input_line;
	if( *buf == '\f' )
	    break;

	for( p = buf; isspace(*p); ++p )
		;
	if( *p == '@' )
	{
	    processing_comment = 1;
	    continue;
	}
	else if( processing_comment )	      /* Previous line was a comment, */
	{				      /* but current line is not.     */
	    processing_comment = 0;
	    if( lines )
		fprintf( output, "\n#line %d \"%s\"\n", Input_line, File_name );
	}
	fputs( buf, output );
    }
    return( feof(Input_file) );
}
