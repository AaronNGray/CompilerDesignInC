/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>

#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "parser.h"
/*----------------------------------------------------------------------*/
extern  void make_yy_stok    P(( void ));			/* public */
extern  void make_token_file P(( void ));
/*----------------------------------------------------------------------*/
PUBLIC	void	make_yy_stok()
{
    /* This subroutine generates the Yy_stok[] array that's
     * indexed by token value and evaluates to a string
     * representing the token name. Token values are adjusted
     * so that the smallest token value is 1 (0 is reserved
     * for end of input).
     */

    register int  i;

    static char	*the_comment[] =
    {
	"Yy_stok[] is used for debugging and error messages. It is indexed",
	"by the internal value used for a token (as used for a column index in",
	"the transition matrix) and evaluates to a string naming that token.",
	NULL
    };

    comment( Output, the_comment );

    output(  "char  *Yy_stok[] =\n{\n" );
    output(   "\t/*   0 */   \"_EOI_\",\n" );

    for( i = MINTERM; i <= Cur_term; i++ )
    {
	output( "\t/* %3d */   \"%s\"",  (i-MINTERM)+1, Terms[i]->name );
	if( i != Cur_term )
	    outc( ',' );

	if( (i & 0x1) == 0  ||  i == Cur_term )		/* Newline for every */
	    outc( '\n' );				/* other element     */
    }

    output(  "};\n\n");
}
/*----------------------------------------------------------------------*/
PUBLIC void	make_token_file()
{
    /* This subroutine generates the yytokens.h file. Tokens have
     * the same value as in make_yy_stok(). A special token
     * named _EOI_ (with a value of 0) is also generated.
     */

    FILE  *tokfile;
    int   i;

    if( !(tokfile = fopen( TOKEN_FILE , "w") ))
	error( FATAL, "Can't open %s\n", TOKEN_FILE );

    D( else if( Verbose )				)
    D( 	  printf("Generating %s\n", TOKEN_FILE );	)

    fprintf( tokfile, "#define _EOI_      0\n");

    for( i = MINTERM; i <= Cur_term; i++ )
	fprintf( tokfile, "#define %-10s %d\n",
					Terms[i]->name, (i-MINTERM)+1 );
    fclose( tokfile );
}
