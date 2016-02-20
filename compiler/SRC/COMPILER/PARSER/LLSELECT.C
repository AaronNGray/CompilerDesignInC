/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>

#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "parser.h"

/* LLSELECT.C	Compute LL(1) select sets for all productions in a
 *		symbol table. The FIRST and FOLLOW sets must be computed
 *		before select() is called. These routines are not used
 *		by occs.
 */
/*----------------------------------------------------------------------*/
void find_select_set	P(( SYMBOL *lhs ));	/* local  */
void select		P(( void        ));	/* public */
/*----------------------------------------------------------------------*/
PRIVATE	void find_select_set( lhs )
SYMBOL	*lhs;				/* left-hand side of production */
{
    /* Find the LL(1) selection set for all productions attached to the
     * indicated left-hand side (lhs). The first_rhs() call puts the FIRST sets
     * for the initial symbols in prod->rhs into the select set. It returns true
     * only if the entire right-hand side was nullable (EPSILON was an element
     * of the FIRST set of every symbol on the right-hand side.
     *
     * The ... in the prototype takes care of a bug in Borland C, which
     * is too picky about argument types when functions are passed as
     * pointers. Additional arguments are actually not used here.
     */

    PRODUCTION  *prod;

    for( prod = lhs->productions; prod ; prod = prod->next )
    {
	if( first_rhs( prod->select, prod->rhs, prod->rhs_len ) )
	    UNION( prod->select, lhs->follow );

	REMOVE( prod->select, EPSILON );
    }
}
/*----------------------------------------------------------------------*/
PUBLIC	void select( )
{
    /* Compute LL(1) selection sets for all productions in the grammar */

    if( Verbose )
	printf("Finding LL(1) select sets\n");

    ptab( Symtab, (ptab_t) find_select_set, NULL, 0 );
}
