/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/l.h>
#include <tools/c-code.h>
#include <tools/compiler.h>
#include "symtab.h"
#include "value.h"
#include "proto.h"
#include "label.h"

/*  VALUE.C:	Routines to manipulate lvalues and rvalues. */

PRIVATE value *Value_free = NULL;
#define VCHUNK	8		/* Allocate this many structure at a time. */
/*----------------------------------------------------------------------*/
value	*new_value()
{
    value *p;
    int   i;

    if( !Value_free )
    {
	if( !(Value_free = (value *) malloc( sizeof(value) * VCHUNK )) )
	{
	    yyerror("INTERNAL, new_value: Out of memory\n");
	    exit( 1 );
	}

	for( p = Value_free, i = VCHUNK; --i > 0; ++p )  /* Executes VCHUNK-1 */
	    p->type = (link *)( p+1 );			 /*            times. */

	p->type = NULL ;
    }

    p 	       = Value_free;
    Value_free = (value *) Value_free->type;
    memset( p, 0, sizeof(value) );
    return p;
}

/*----------------------------------------------------------------------*/

void	discard_value( p )
value   *p;		 			/* Discard a value structure  */
{				 		/* and any associated links.  */
    if( p )
    {
	if( p->type )
	    discard_link_chain( p->type );

	p->type = (link *)Value_free ;
	Value_free = p;
    }
}

/*----------------------------------------------------------------------*/

char	*shift_name( val, left )
value	*val;
int	left;
{
    /* Shift the name one character to the left or right. The string might get
     * truncated on a right shift. Returns pointer to shifted string.
     * Shifts an & into the new slot on a right shift.
     */

    if( left )
	memmove( val->name,   val->name+1, sizeof(val->name) - 1 );
    else
    {
	memmove( val->name+1, val->name,   sizeof(val->name) - 1 );
	val->name[ sizeof(val->name) - 1 ] = '\0' ;
	val->name[0] = '&';
    }

    return val->name;
}

/*----------------------------------------------------------------------*/

char	*rvalue( val )
value	*val;
{
    /* If val is an rvalue, do nothing and return val->name. If it's an lvalue,
     * convert it to an rvalue and return the string necessary to access the
     * rvalue. If it's an rvalue constant, return a string representing the
     * number (and also put that string into val->name).
     */

    static char buf[ NAME_MAX + 2 ] = "*" ;

    if( !val->lvalue )				/* It's an rvalue already. */
    {
	if( IS_CONSTANT(val->etype) )
	    strcpy( val->name, CONST_STR(val) );
	return val->name;
    }
    else
    {
	val->lvalue = 0;
	if( * val->name == '&' )		/* *& cancels */
	{
	    shift_name(val, LEFT);
	    return val->name;
	}
	else
	{
	    strncpy( &buf[1], val->name, NAME_MAX+1 );
	    return buf;
	}
    }
}

char	*rvalue_name( val )
value	*val;
{
    /* Returns a copy of the rvalue name without actually converting val to an
     * rvalue. The copy remains valid only until the next rvalue_name call.
     */

    static char buf[ (NAME_MAX * 2) + 1 ];	/* sizeof(val->name) + 1 */

    if( !val->lvalue )
    {
	if( IS_CONSTANT(val->etype) ) strcpy( buf, CONST_STR(val) );
	else			      strcpy( buf, val->name	  );
    }
    else
    {
	if( * val->name == '&' ) strcpy ( buf,        val->name + 1 );
	else		 	 sprintf( buf, "*%s", val->name	    );
    }
    return buf;
}

value *tmp_create( type, add_pointer )
link  *type;	      /* Type of temporary, NULL to create an int.	  */
int   add_pointer;    /* If true, add pointer declarator to front of type */
{		      /* before creating the temporary.			  */

    /* Create a temporary variable and return a pointer to it. It's an rvalue
     * by default.
     */

    value *val;
    link  *lp;

    val         = new_value();
    val->is_tmp = 1;

    if( type )					/* Copy existing type. */
	val->type  = clone_type( type, &lp );
    else					/* Make an integer.    */
    {
	lp	    = new_link();
	lp->class   = SPECIFIER;
	lp->NOUN    = INT;
	val->type   = lp;
    }

    val->etype  = lp;
    lp->SCLASS  = AUTO;			    /* It's an auto now, regardless   */
					    /* of the previous storage class. */
    if( add_pointer )
    {
	lp	     = new_link();
	lp->DCL_TYPE = POINTER;
	lp->next     = val->type;
	val->type    = lp;
    }

    val->offset = tmp_alloc( get_size( val->type ) );
    sprintf(val->name, "%s( T(%d) )", get_prefix(val->type), (val->offset + 1));
    return( val );
}

/*----------------------------------------------------------------------*/

char	*get_prefix( type )
link	*type;
{
    /* Return the first character of the LP(), BP(), WP(), etc., directive
     * that accesses a variable of the given type. Note that an array or
     * structure type is assumed to be a pointer to the first cell at run time.
     */

    int  c;

    if( type )
    {
	if( type->class == DECLARATOR )
	{
	    switch( type->DCL_TYPE )
	    {
	    case ARRAY:	    return( get_prefix( type->next ) );
	    case FUNCTION:  return PTR_PREFIX;
	    case POINTER:
		c = *get_prefix( type->next );

		if     ( c == *BYTE_PREFIX  )  return BYTEPTR_PREFIX;
		else if( c == *WORD_PREFIX  )  return WORDPTR_PREFIX;
		else if( c == *LWORD_PREFIX )  return LWORDPTR_PREFIX;
		else			       return PTRPTR_PREFIX;
	    }
	}
	else
	{
	    switch( type->NOUN )
	    {
	    case INT:	    return (type->LONG) ?  LWORD_PREFIX : WORD_PREFIX;
	    case CHAR:	    return BYTE_PREFIX;
	    case STRUCTURE: return BYTEPTR_PREFIX ;
	    }
	}
    }
    yyerror("INTERNAL, get_prefix: Can't process type %s.\n", type_str(type) );
    exit( 1 );
    BCC( return NULL; )	/* Shouldn't get here, but keep the compiler happy */
}

/*----------------------------------------------------------------------*/

value   *tmp_gen( tmp_type, src )
link	*tmp_type;	/* type of temporary taken from here		 */
value   *src;		/* source variable that is copied into temporary */
{
    /* Create a temporary variable of the indicated type and then generate
     * code to copy src into it. Return a pointer to a "value" for the temporary
     * variable. Truncation is done silently; you may want to add a lint-style
     * warning message in this situation, however. Src is converted to an
     * rvalue if necessary, and is released after being copied. If tmp_type
     * is NULL, an int is created.
     */

    value  *val;			/* temporary variable		      */
    char   *reg;

    if( !the_same_type( tmp_type, src->type, 1) )
    {
	/* convert_type() copies src to a register and does any necessary type
	 * conversion. It returns a string that can be used to access the
	 * register. Once the src has been copied, it can be released, and
	 * a new temporary (this time of the new type) is created and
	 * initialized from the register.
	 */

	reg = convert_type( tmp_type, src );
	release_value( src );
	val = tmp_create( IS_CHAR(tmp_type) ? NULL : tmp_type, 0 );
	gen( "=",  val->name, reg );
    }
    else
    {
	val = tmp_create( tmp_type, 0 );
	gen( "=", val->name, rvalue(src) );
	release_value( src );
    }
    return val;
}

/*----------------------------------------------------------------------*/


char    *convert_type( targ_type, src )
link	*targ_type;			/* type of target object	   */
value	*src;				/* source to be converted	   */
{
    int    dsize;			/* src size, dst size		   */
    static char reg[16];		/* place to assemble register name */

    /* This routine should only be called if the target type (in targ_type)
     * and the source type (in src->type) disagree. It generates code to copy
     * the source into a register and do any type conversion. It returns a
     * string that references the register. This string should be used
     * immediately as the source of an assignment instruction, like this:
     *
     *	    gen( "=",  dst_name, convert_type( dst_type, src ); )
     */

    sprintf( reg, "r0.%s",  get_suffix(src->type) );		/* copy into */
    gen    ( "=",  reg,	    rvalue(src)		  );		/* register. */

    if( (dsize = get_size(targ_type)) > get_size(src->type) )
    {
	if( src->etype->UNSIGNED )			       /* zero fill */
	{
	    if( dsize == 2 ) gen( "=", "r0.b.b1",   "0" );
	    if( dsize == 4 ) gen( "=", "r0.w.high", "0" );
	}
	else						       /* sign extend */
	{
	    if( dsize == 2 ) gen( "ext_low",  "r0" );
	    if( dsize == 4 ) gen( "ext_word", "r0" );
	}
    }

    sprintf( reg, "r0.%s",  get_suffix(targ_type) );
    return reg;
}

/*----------------------------------------------------------------------*/

PUBLIC	int	get_size( type )
link	*type;
{
    /* Return the number of bytes required to hold the thing referenced by
     * get_prefix().
     */

    if( type )
    {
	if( type->class == DECLARATOR )
	    return (type->DCL_TYPE == ARRAY) ? get_size(type->next) : PSIZE ;
	else
	{
	    switch( type->NOUN )
	    {
	    case INT:	    return (type->LONG) ?  LSIZE : ISIZE;
	    case CHAR:
	    case STRUCTURE: return CSIZE;
	    }
	}
    }
    yyerror("INTERNAL, get_size: Can't size type: %s.\n", type_str(type));
    exit(1);
    BCC( return NULL; )	/* Shouldn't get here, but keep the compiler happy */
}

/*----------------------------------------------------------------------*/

char	*get_suffix( type )
link	*type;
{
    /* Returns the string for an operand that gets a number of the indicated
     * type out of a register. (It returns the xx in : r0.xx). "pp" is returned
     * for pointers, arrays, and structures--get_suffix() is used only for
     * temporary-variable manipulation, not declarations. If an an array or
     * structure declarator is a component of a temporary-variable's type chain,
     * then that declarator actually represents a pointer at run time. The
     * returned string is, at most, five characters long.
     */

    if( type )
    {
	if( type->class == DECLARATOR )
	    return "pp";
	else
	    switch( type->NOUN )
	    {
	    case INT:	    return (type->LONG) ?  "l" : "w.low";
	    case CHAR:
	    case STRUCTURE: return "b.b0";
	    }
    }
    yyerror("INTERNAL, get_suffix: Can't process type %s.\n", type_str(type) );
    exit( 1 );
    BCC( return NULL; )	/* Shouldn't get here, but keep the compiler happy */
}

/*----------------------------------------------------------------------*/

void	release_value( val )
value	*val;			/* Discard a value, first freeing any space   */
{				/* used for an associated temporary variable. */
    if( val )
    {
	if( val->is_tmp )
	    tmp_free( val->offset );
	discard_value( val );
    }
}

value  *make_icon( yytext, numeric_val )
char	*yytext;
int	numeric_val;
{
    /* Make an integer-constant rvalue. If yytext is NULL, then numeric_val
     * holds the numeric value, otherwise the second argument is not used and
     * the string in yytext represents the constant.
     */

    value *vp;
    link  *lp;
    char  *p;

    vp		= make_int();
    lp		= vp->type;
    lp->SCLASS  = CONSTANT;

    if( !yytext )
	lp->V_INT = numeric_val;

    else if( *yytext == '\'' )
    {
	++yytext;			/* Skip the quote. */
	lp->V_INT = esc( &yytext );
    }
    else				/* Initialize the const_val field     */
    {					/* based on the input type. stoul()   */
					/* converts a string to unsigned long.*/
	for( p = yytext; *p ; ++p )
	{
	    if     ( *p=='u' || *p=='U' ) { lp->UNSIGNED = 1; }
	    else if( *p=='l' || *p=='L' ) { lp->LONG     = 1; }
	}
	if( lp->LONG )
	    lp->V_ULONG = stoul( &yytext );
	else
	    lp->V_UINT  = (unsigned int) stoul( &yytext );
    }
    return vp;
}

/*----------------------------------------------------------------------*/

value  *make_int()
{					/* Make an unnamed integer rvalue. */
    link  *lp;
    value *vp;

    lp		= new_link();
    lp->class   = SPECIFIER;
    lp->NOUN    = INT;
    vp		= new_value();		/* It's an rvalue by default. */
    vp->type	= vp->etype = lp;
    return vp;
}
value	*make_scon()
{
    link *p;
    value *synth;
    static unsigned label = 0;

    synth 	 = new_value();
    synth->type  = new_link();

    p		 = synth->type;
    p->DCL_TYPE  = POINTER;
    p->next      = new_link();

    p 		 = p->next;
    p->class     = SPECIFIER;
    p->NOUN      = CHAR;
    p->SCLASS    = CONSTANT;
    p->V_INT     = ++label;

    synth->etype  = p;
    sprintf( synth->name, "%s%d", L_STRING, label );
    return synth;
}
