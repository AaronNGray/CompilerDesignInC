/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <tools/debug.h>
#include <tools/l.h>

void	main( argc, argv )
char	**argv;
int	argc;
{
    /* A default main module to test the lexical analyzer.  */

    int yylex( void );
    if( argc == 2 )
	ii_newfile( argv[1] );
    while( yylex() )
	;
    exit( 0 );
}
