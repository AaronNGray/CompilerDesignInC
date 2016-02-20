/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/l.h>
#include <tools/compiler.h>
#include <tools/c-code.h>
#include <tools/occs.h>
#include "symtab.h"
#include "proto.h"
#include "label.h"

/* LOCAL.C Subroutines in this file take care of local-variable management.  */

PRIVATE int	Offset = 0 ;	/* Offset from the frame pointer (which also */
				/* marks the base of the automatic-variable  */
				/* region of the stack frame) to the most    */
				/* recently allocated variable. Reset to 0   */
				/* by loc_reset() at the head of every sub-  */
				/* routine.				     */
/*----------------------------------------------------------------------*/
void	loc_reset()
{
    /* Reset everything back to the virgin state. Call this subroutine just */
    /* before processing the outermost compound statement in a subroutine.  */

    Offset = 0 ;
}
/*----------------------------------------------------------------------*/
int	loc_var_space()
{
    /* Return the total cumulative size of the temporary-variable region in
     * stack elements (not bytes). This call outputs the value of the macro
     * that specifies the variable-space size in the link instruction. Calling
     * loc_reset() also resets the return value of this subroutine to zero.
     */

    return( (Offset + (SWIDTH-1)) / SWIDTH );
}
/*----------------------------------------------------------------------*/
#ifdef __TURBOC__
#pragma argsused
#endif

void	figure_local_offsets( sym, funct_name )
symbol	*sym;
char	*funct_name;     /* not used, but keep it to help in debugging */
{
    /* Add offsets for all local automatic variables in the sym list.  */

    for(; sym ; sym = sym->next )
	if( !IS_FUNCT( sym->type ) && !sym->etype->STATIC )
	    loc_auto_create( sym );
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void	loc_auto_create( sym )
symbol	*sym;
{
    /* Create a local automatic variable, modifying the "rname" field of "sym"
     * to hold a string that can be used as an operand to reference that
     * variable. This name is a correctly aligned reference of the form:
     *
     *			fp + offset
     *
     * Local variables are packed as well as possible into the stack frame,
     * though, as was the case with structures, some padding may be necessary
     * to get things aligned properly.
     */

    int	align_size = get_alignment( sym->type );

    Offset += get_sizeof( sym->type );		/* Offset from frame pointer */
						/* to variable.              */

    while( Offset % align_size )		/* Add any necessary padding */
	++Offset;				/* to guarantee alignment.   */

    sprintf( sym->rname, "fp-%d", Offset );	/* Create the name.          */
    sym->etype->SCLASS = AUTO;
}
/*----------------------------------------------------------------------*/
void	create_static_locals( sym, funct_name )
symbol	*sym;
char	*funct_name;
{
    /* Generate definitions for local, static variables in the sym list. */

    for(; sym ; sym = sym->next )
	if( !IS_FUNCT( sym->type ) && sym->etype->STATIC )
	    loc_static_create ( sym, funct_name );
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
void	loc_static_create( sym, funct_name )
symbol	*sym;
char	*funct_name;
{
    static unsigned val;	    /* Numeric component of arbitrary name. */

    sprintf( sym->rname, "%s%d", L_VAR, val++ );
    sym->etype->SCLASS = FIXED ;
    sym->etype->OCLASS = PRI   ;

    var_dcl( yybss, PRI, sym, ";" );
    yybss( "\t/* %s [%s(), static local] */\n", sym->name, funct_name );
}
/*----------------------------------------------------------------------*/

void	remove_symbols_from_table( sym )
symbol	*sym;
{
    /* Remove all symbols in the list from the table.  */

    symbol *p;

    for( p = sym; p ; p = p->next )
	if( !p->duplicate )
	    delsym( Symbol_tab, p );
	else
	{
	    yyerror("INTERNAL, remove_symbol: duplicate sym. in cross-link\n");
	    exit( 1 );
	}
}
