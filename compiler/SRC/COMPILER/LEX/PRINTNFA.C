/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include "nfa.h"

PRIVATE  void	printccl	P(( SET*		));
PRIVATE  char	*plab		P(( NFA*, NFA*		));
PUBLIC   void	print_nfa	P(( NFA*, int, NFA*	));

/*--------------------------------------------------------------
 *  PRINTNFA.C	Routine to print out a NFA structure in human-readable form.
 */

PRIVATE void	printccl( set )
SET	*set;
{
    static int	i;

    putchar('[');
    for( i = 0 ; i <= 0x7f; i++ )
    {
	if( TEST(set, i) )
	{
	    if( i < ' ' )
		printf( "^%c", i + '@' );
	    else
		printf( "%c", i );
	}
    }

    putchar(']');
}

/*--------------------------------------------------------------*/

PRIVATE  char	*plab( nfa, state )
NFA	*nfa, *state ;
{
    /* Return a pointer to a buffer containing the state number. The buffer is
     * overwritten on each call so don't put more than one plab() call in an
     * argument to printf().
     */

    static char	buf[ 32 ];

    if( !nfa || !state )
	    return("--");

    sprintf( buf, "%2ld", (long)(state - nfa) );
    return ( buf );
}

/*--------------------------------------------------------------*/

PUBLIC  void	print_nfa( nfa, len, start )
NFA	*nfa, *start;
int	len;
{
    NFA	*s = nfa ;

    printf( "\n----------------- NFA ---------------\n" );

    for(; --len >= 0 ; nfa++ )
    {
	printf( "NFA state %s: ", plab(s,nfa) );

	if( !nfa->next )
	    printf("(TERMINAL)");
	else
	{
	    printf( "--> %s ",  plab(s, nfa->next ) );
	    printf( "(%s) on ", plab(s, nfa->next2) );

	    switch( nfa->edge )
	    {
	    case CCL:     printccl( nfa->bitset  	);	break;
	    case EPSILON: printf  ( "EPSILON "   	);	break;
	    default:      pchar   ( nfa->edge, stdout	);	break;
	    }
	}

	if( nfa == start )
	    printf(" (START STATE)");

	if( nfa->accept )
	    printf(" accepting %s<%s>%s", nfa->anchor & START ? "^" : "",
					      nfa->accept,
					      nfa->anchor & END   ? "$" : "" );
	printf( "\n" );
    }
    printf( "\n-------------------------------------\n" );
}
