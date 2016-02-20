/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/l.h>
#include <tools/compiler.h>
#include <tools/c-code.h>
#include "symtab.h"
#include "value.h"
#include "proto.h"
#include "label.h"
#include "switch.h"

/*----------------------------------------------------------------------*/

PUBLIC	stab  *new_stab( val, label )
value	*val;
int	label;
{
    /* Allocate a new switch table and return a pointer to it. Use free() to
     * discard the table. Val is the value to switch on, it is converted to
     * an rvalue, if necessary, and the name is stored.
     */

    stab *p;

    if( !(p = (stab *) malloc(sizeof(stab)) ))
    {
	yyerror("No memory for switch\n");
	exit( 1 );
    }

    p->cur	  = p->table ;
    p->stab_label = label;
    p->def_label  = 0;
    strncpy( p->name, rvalue(val), sizeof(p->name) );
    return p;
}

/*----------------------------------------------------------------------*/

PUBLIC	void	add_case( p, on_this, go_here )
stab	*p;
int	on_this;
int	go_here;
{
    /* Add a new case to the stab at top of stack. The `cur` field identifies
     * the next available slot in the dispatch table.
     */

    if( p->cur  >  &(p->table[CASE_MAX-1]) )
	yyerror("Too many cases in switch\n");
    else
    {
	p->cur->on_this = on_this ;
	p->cur->go_here = go_here;
	++( p->cur );
    }
}

/*----------------------------------------------------------------------*/

PUBLIC	void	add_default_case( p, go_here )
stab	*p;
int	go_here;
{
    /* Add the default case to the current switch by remembering its label */

    if( p->def_label )
	yyerror("Only one default case permitted in switch\n");

    p->def_label = go_here;
}

/*----------------------------------------------------------------------*/

PUBLIC	void	gen_stab_and_free_table( p )
stab *p;
{
    /* Generate the selector code at the bottom of the switch. This routine is
     * emitting what amounts to a series of if/else statements. It could just
     * as easily emit a dispatch or jump table and the code necessary to process
     * that table at run time, however.
     */

    case_val	*cp;

    gen( "goto%s%d", L_SWITCH, p->stab_label + 1 );
    gen( ":%s%d",    L_SWITCH, p->stab_label     );

    for( cp = p->table ; cp < p->cur ; ++cp )
    {
	gen( "EQ%s%d", 	 p->name,  cp->on_this );
	gen( "goto%s%d", L_SWITCH, cp->go_here );
    }

    if( p->def_label )
	gen( "goto%s%d", L_SWITCH, p->def_label );

    gen( ":%s%d", L_SWITCH, p->stab_label + 1 );
    free( p );
}
