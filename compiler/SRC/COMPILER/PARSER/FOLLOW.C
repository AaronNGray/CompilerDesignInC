/*@A (C) 1992 Allen I. Holub                                                */

#include <stdio.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "parser.h"
#include "llout.h"

/* FOLLOW.C		Compute FOLLOW sets for all productions in a
 *			symbol table. The FIRST sets must be computed
 *			before follow() is called.
 */

void	follow_closure	P(( SYMBOL *lhs ));		/* local */
void	remove_epsilon	P(( SYMBOL *lhs ));
void	init		P(( SYMBOL *lhs ));
void	follow		P(( void        ));		/* public */

/*----------------------------------------------------------------------*/
PRIVATE int	Did_something;
/*----------------------------------------------------------------------*/
PUBLIC	void follow()
{
    D( int pass = 0; 			   	 )
    D( printf( "Initializing FOLLOW sets\n" ); )

    ptab( Symtab, (ptab_t) init, NULL, 0 );

    do {
	/* This loop makes several passes through the entire grammar, adding
	 * FOLLOW sets. The follow_closure() routine is called for each grammar
	 * symbol, and sets Did_something to true if it adds any elements to
	 * existing FOLLOW sets.
	 */

	D(  printf( "Closure pass %d\n", ++pass ); )
	D( fprintf( stderr, "%d\n",        pass ); )

	Did_something = 0;
	ptab( Symtab, (ptab_t) follow_closure, NULL, 0 );

    } while( Did_something );

    /* This last pass is just for nicety and could probably be eliminated (may
     * as well do it right though). Strictly speaking, FOLLOW sets shouldn't
     * contain epsilon. Nonetheless, it was much easier to just add epsilon in
     * the previous steps than try to filter it out there. This last pass just
     * removes epsilon from all the FOLLOW sets.
     */

    ptab( Symtab, (ptab_t) remove_epsilon, NULL, 0 );
    D( printf("Follow set computation done\n"); )
}
/*----------------------------------------------------------------------*/
PRIVATE void init( lhs )
SYMBOL	*lhs; 			/* Current left-hand side */
{
    /* Initialize the FOLLOW sets. This procedure adds to the initial follow set
     * of each production those elements that won't change during the closure
     * process. Note that in all the following cases, actions are just ignored.
     *
     * (1) FOLLOW(start) contains end of input ($).
     *
     * (2) Given s->...xY... where x is a nonterminal and Y is
     * 	   a terminal, add Y to FOLLOW(x). x and Y can be separated
     *	   by any number (including 0) of nullable nonterminals or actions.
     *
     * (3) Given x->...xy... where x and y are both nonterminals,
     *	   add FIRST(y) to FOLLOW(x). Again, x and y can be separated
     *	   by any number of nullable nonterminals or actions.
     */

    PRODUCTION  *prod; 	 /* Pointer to one production 		 */
    SYMBOL	**x;  	 /* Pointer to one element of production */
    SYMBOL	**y;

    D( printf("%s:\n", lhs->name); )

    if( !ISNONTERM(lhs) )
	return;

    if( lhs == Goal_symbol )
    {
	D( printf( "\tAdding _EOI_ to FOLLOW(%s)\n", lhs->name ); )
	ADD( lhs->follow, _EOI_ );
    }

    for( prod = lhs->productions; prod ; prod = prod->next )
    {
	for( x = prod->rhs ; *x ; x++ )
	{
	    if( ISNONTERM(*x) )
	    {
		for( y = x + 1 ; *y ; ++y )
		{
		    if( ISACT(*y) )
			continue;

		    if( ISTERM(*y) )
		    {
			D( printf("\tAdding %s ",    (*y)->name ); )
			D( printf("to FOLLOW(%s)\n", (*x)->name ); )

			ADD( (*x)->follow, (*y)->val );
			break;
		    }
		    else
		    {
			UNION( (*x)->follow, (*y)->first );
			if( !NULLABLE(*y) )
			    break;
		    }
		}
	    }
	}
    }
}
/*----------------------------------------------------------------------*/
PRIVATE	void follow_closure( lhs )
SYMBOL	*lhs;
{
    /* Adds elements to the FOLLOW sets using the following rule:
     *
     *    Given s->...x  or  s->...x... where all symbols following
     *	  x are nullable nonterminals or actions, add FOLLOW(s) to FOLLOW(x).
     */

    PRODUCTION  *prod;  /* Pointer to one production side	*/
    SYMBOL	**x;	/* Pointer to one element of production */

    D( printf("%s:\n", lhs->name ); )

    if( ISACT(lhs) || ISTERM(lhs) )
	return;

    for( prod = lhs->productions; prod ; prod = prod->next )
    {
	for( x = prod->rhs + prod->rhs_len; --x >= prod->rhs ; )
	{
	    if( ISACT(*x) )
		continue;

	    if( ISTERM(*x) )
		break;

	    if( !subset( (*x)->follow, lhs->follow ) )
	    {
		D( printf("\tAdding FOLLOW(%s) ", lhs->name  ); )
		D( printf("to FOLLOW(%s)\n",      (*x)->name ); )

		UNION( (*x)->follow, lhs->follow );
		Did_something = 1;
	    }
	    if( ! NULLABLE(*x) )
		break;
	}
    }
}
/*----------------------------------------------------------------------*/
PRIVATE void  remove_epsilon( lhs )
SYMBOL	*lhs;
{
    /* Remove epsilon from the FOLLOW sets. The presence of epsilon is a
     * side effect of adding FIRST sets to the FOLLOW sets willy nilly.
     */

    if( ISNONTERM(lhs) )
	REMOVE( lhs->follow, EPSILON );
}
