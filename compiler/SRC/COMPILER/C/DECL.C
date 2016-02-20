/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/l.h>
#include <tools/compiler.h>
#include <tools/c-code.h>
#include <tools/occs.h>

#include "symtab.h"
#include "value.h"
#include "proto.h"

/* DECL.C	This file contains support subroutines for those actions in c.y
 *		that deal with declarations.
 *----------------------------------------------------------------------
 */

PUBLIC	link	  *new_class_spec( first_char_of_lexeme )
int	first_char_of_lexeme;
{
    /* Return a new specifier link with the sclass field initialized to hold
     * a storage class, the first character of which is passed in as an argument
     * ('e' for extern, 's' for static, and so forth).
     */

    link *p  = new_link();
    p->class = SPECIFIER;
    set_class_bit( first_char_of_lexeme, p );
    return p;
}

/*----------------------------------------------------------------------*/

PUBLIC	void	set_class_bit( first_char_of_lexeme, p )
int	first_char_of_lexeme;
link	*p;
{
      /* Change the class of the specifier pointed to by p as indicated by the
       * first character in the lexeme. If it's 0, then the defaults are
       * restored (fixed, nonstatic, nonexternal). Note that the TYPEDEF
       * class is used here only to remember that the input storage class
       * was a typedef, the tdef field in the link is set true (and the storage
       * class is cleared) before the entry is added to the symbol table.
       */

      switch( first_char_of_lexeme )
      {
      case 0:	p->SCLASS = FIXED;
		p->STATIC = 0;
		p->EXTERN = 0;
		break;

      case 't': p->SCLASS  = TYPEDEF  ;	break;
      case 'r': p->SCLASS  = REGISTER ;	break;
      case 's': p->STATIC  = 1        ;	break;
      case 'e': p->EXTERN  = 1        ;	break;

      default : yyerror("INTERNAL, set_class_bit: bad storage class '%c'\n",
							  first_char_of_lexeme);
		exit( 1 );
		break;
      }
}

/*----------------------------------------------------------------------*/

PUBLIC	link	*new_type_spec( lexeme )
char	*lexeme;
{
    /* Create a new specifier and initialize the type according to the indicated
     * lexeme. Input lexemes are: char const double float int long short
     *				  signed unsigned void volatile
     */

    link *p  = new_link();
    p->class = SPECIFIER;

    switch( lexeme[0] )
    {
    case 'c': 	if( lexeme[1]=='h' )			/* char | const	   */
		    p->NOUN = CHAR ;			/* (Ignore const.) */
		break;
    case 'd':						/* double	 */
    case 'f':	yyerror("No floating point\n");		/* float	 */
		break;

    case 'i':	p->NOUN	    = INT;	break;		/* int		 */
    case 'l':	p->LONG	    = 1;	break;		/* long		 */
    case 'u': 	p->UNSIGNED = 1;	break;		/* unsigned	 */

    case 'v':	if( lexeme[2] == 'i' )			/* void | volatile */
		    p->NOUN = VOID;			/* ignore volatile */
		break;
    case 's': 	break;					/* short | signed */
    }							/* ignore both 	  */

    return p;
}
void	add_spec_to_decl( p_spec, decl_chain )
link	*p_spec;
symbol	*decl_chain;
{
    /* p_spec is a pointer either to a specifier/declarator chain created
     * by a previous typedef or to a single specifier. It is cloned and then
     * tacked onto the end of every declaration chain in the list pointed to by
     * decl_chain. Note that the memory used for a single specifier, as compared
     * to a typedef, may be freed after making this call because a COPY is put
     * into the symbol's type chain.
     *
     * In theory, you could save space by modifying all declarators to point
     * at a single specifier. This makes deletions much more difficult, because
     * you can no longer just free every node in the chain as it's used. The
     * problem is complicated further by typedefs, which may be declared at an
     * outer level, but can't be deleted when an inner-level symbol is
     * discarded. It's easiest to just make a copy.
     *
     * Typedefs are handled like this: If the incoming storage class is TYPEDEF,
     * then the typedef appeared in the current declaration and the tdef bit is
     * set at the head of the cloned type chain and the storage class in the
     * clone is cleared; otherwise, the clone's tdef bit is cleared (it's just
     * not copied by clone_type()).
     */

    link *clone_start, *clone_end ;

    for( ; decl_chain ; decl_chain = decl_chain->next )
    {
	if( !(clone_start = clone_type(p_spec, &clone_end)) )
	{
	    yyerror("INTERNAL, add_typedef_: Malformed chain (no specifier)\n");
	    exit( 1 );
	}
	else
	{
	    if( !decl_chain->type )			  /* No declarators. */
		decl_chain->type = clone_start ;
	    else
		decl_chain->etype->next = clone_start;

	    decl_chain->etype = clone_end;

	    if( IS_TYPEDEF(clone_end) )
	    {
		set_class_bit( 0, clone_end );
		decl_chain->type->tdef = 1;
	    }
	}
    }
}
void	add_symbols_to_table( sym )
symbol	*sym;
{
    /* Add declarations to the symbol table.
     *
     * Serious redefinitions (two publics, for example) generate an error
     * message. Harmless redefinitions are processed silently. Bad code is
     * generated when an error message is printed. The symbol table is modified
     * in the case of a harmless duplicate to reflect the higher precedence
     * storage class: (public == private) > common > extern.
     *
     * The sym->rname field is modified as if this were a global variable (an
     * underscore is inserted in front of the name). You should add the symbol
     * chains to the table before modifying this field to hold stack offsets
     * in the case of local variables.
     */

    symbol *exists;		/* Existing symbol if there's a conflict.    */
    int    harmless;
    symbol *new;

    for(new = sym; new ; new = new->next )
    {
	exists = (symbol *) findsym(Symbol_tab, new->name);

	if( !exists || exists->level != new->level )
	{
	    sprintf ( new->rname, "_%1.*s", sizeof(new->rname)-2, new->name);
	    addsym  ( Symbol_tab, new );
	}
	else
	{
	    harmless	   = 0;
	    new->duplicate = 1;

	    if( the_same_type( exists->type, new->type, 0) )
	    {
		if( exists->etype->OCLASS==EXT || exists->etype->OCLASS==COM )
		{
		    harmless = 1;

		    if( new->etype->OCLASS != EXT )
		    {
		    	exists->etype->OCLASS = new->etype->OCLASS;
		    	exists->etype->SCLASS = new->etype->SCLASS;
		    	exists->etype->EXTERN = new->etype->EXTERN;
		    	exists->etype->STATIC = new->etype->STATIC;
		    }
		}
	    }
	    if( !harmless )
		yyerror("Duplicate declaration of %s\n", new->name );
	}
    }
}

/*----------------------------------------------------------------------*/

void	figure_osclass( sym )
symbol	*sym;
{
    /* Go through the list figuring the output storage class of all variables.
     * Note that if something is a variable, then the args, if any, are a list
     * of initializers. I'm assuming that the sym has been initialized to zeros;
     * at least the OSCLASS field remains unchanged for nonautomatic local
     * variables, and a value of zero there indicates a nonexistent output class.
     */

    for( ; sym ; sym = sym->next )
    {
	if( sym->level == 0 )
	{
	    if( IS_FUNCT( sym->type ) )
	    {
		if	( sym->etype->EXTERN )	sym->etype->OCLASS = EXT;
		else if	( sym->etype->STATIC )	sym->etype->OCLASS = PRI;
		else				sym->etype->OCLASS = PUB;
	    }
	    else
	    {
		if	( sym->etype->STATIC )	sym->etype->OCLASS = PRI;
		if      ( sym->args	     )	sym->etype->OCLASS = PUB;
		else				sym->etype->OCLASS = COM;
	    }
	}
	else if( sym->type->SCLASS == FIXED )
	{
	    if      (  IS_FUNCT ( sym->type ))	sym->etype->OCLASS = EXT;
	    else if (! IS_LABEL ( sym->type ))	sym->etype->OCLASS = PRI;
	}
    }
}

/*----------------------------------------------------------------------*/

void	generate_defs_and_free_args( sym )
symbol	*sym;
{
    /* Generate global-variable definitions, including any necessary
     * initializers. Free the memory used for the initializer (if a variable)
     * or argument list (if a function).
     */

    for( ; sym ; sym = sym->next )
    {
	if( IS_FUNCT(sym->type) )
	{
	    /* Print a definition for the function and discard arguments
	     * (you'd keep them if prototypes were supported).
	     */

	    yydata( "external\t%s();\n", sym->rname );
	    discard_symbol_chain( sym->args );
	    sym->args = NULL;
	}
	else if( IS_CONSTANT(sym->etype) || sym->type->tdef )
	{
	    continue;
	}
	else if( !sym->args )		/* It's an uninitialized variable. */
	{
	    print_bss_dcl( sym );	/* Print the declaration.	  */
	}
	else				/* Deal with an initializer.	  */
	{
	    var_dcl( yydata, sym->etype->OCLASS, sym, "=" );

	    if( IS_AGGREGATE( sym->type ) )
		yyerror("Initialization of aggregate types not supported\n");

	    else if( !IS_CONSTANT( ((value *)sym->args)->etype ) )
		yyerror("Initializer must be a constant expression\n");

	    else if( !the_same_type(sym->type, ((value *) sym->args)->type, 0) )
		yyerror("Initializer: type mismatch\n");

	    else
		yydata( "%s;\n", CONST_STR( (value *) sym->args ) );

	    discard_value( (value *)(sym->args) );
	    sym->args = NULL;
	}
    }
}

/*----------------------------------------------------------------------*/

symbol	*remove_duplicates( sym )
symbol	*sym;
{
    /* Remove all nodes marked as duplicates from the linked list and free the
     * memory. These nodes should not be in the symbol table. Return the new
     * head-of-list pointer (the first symbol may have been deleted).
     */

    symbol *prev  = NULL;
    symbol *first = sym;

    while( sym )
    {
	if( !sym->duplicate )		    /* Not a duplicate, go to the     */
	{				    /* next list element.	      */
	    prev = sym;
	    sym  = sym->next;
	}
	else if( prev == NULL )		    /* Node is at start of the list.  */
	{
	    first = sym->next;
	    discard_symbol( sym );
	    sym = first;
	}
	else			    	    /* Node is in middle of the list. */
	{
	    prev->next = sym->next;
	    discard_symbol( sym );
	    sym = prev->next;
	}
    }
    return first;
}
void	print_bss_dcl( sym )	/* Print a declaration to the bss segment. */
symbol  *sym;
{
    if( sym->etype->SCLASS != FIXED )
	yyerror( "Illegal storage class for %s", sym->name );
    else
    {
	if( sym->etype->STATIC  &&  sym->etype->EXTERN )
	    yyerror("%s: Bad storage class\n", sym->name );

	var_dcl( yybss, sym->etype->OCLASS, sym, ";\n" );
    }
}

/*----------------------------------------------------------------------*/

PUBLIC	void var_dcl( ofunct, c_code_sclass, sym, terminator )

void   (* ofunct) P((char *,...));  /* output function (yybss or yydata). */
int    c_code_sclass;	/* C-code storage class of symbol.		  */
symbol *sym;		/* Symbol itself.				  */
char   *terminator;	/* Print this string at end of the declaration.	  */
{
    /* Kick out a variable declaration for the current symbol. */

    char suffix[32];
    char *type		  = "" ;
    int  size		  = 1  ;
    link *p		  = sym->type;
    char *storage_class   = (c_code_sclass == PUB) ?  "public"    :
    		            (c_code_sclass == PRI) ?  "private"   :
    		            (c_code_sclass == EXT) ?  "external"  :  "common" ;
    *suffix = '\0';

    if( IS_FUNCT(p) )
    {
	yyerror("INTERNAL, var_dcl: object not a variable\n");
	exit( 1 );
    }

    if( IS_ARRAY(p) )
    {
        for(; IS_ARRAY(p) ; p = p->next )
	    size *= p->NUM_ELE ;

	sprintf( suffix, "[%d]", size );
    }

    if( IS_STRUCT(p) )
    {
	( *ofunct )( "\nALIGN(lword)\n" );
	sprintf( suffix, "[%d]", size * p->V_STRUCT->size );
    }

    if( IS_POINTER(p) )
	type = PTYPE;
    else					/* Must be a specifier. */
	switch( p->NOUN )
	{
	case CHAR:	type = CTYPE;			 break;
	case INT:	type = p->LONG ? LTYPE : ITYPE;	 break;
	case STRUCTURE:	type = STYPE;			 break;
	}

    ( *ofunct )( "%s\t%s\t%s%s%s", storage_class, type,
					    sym->rname, suffix, terminator );
}

int  illegal_struct_def( cur_struct, fields )
structdef *cur_struct;
symbol	  *fields;
{
    /* Return true if any of the fields are defined recursively or if a function
     * definition (as compared to a function pointer) is found as a field.
     */

    for(; fields; fields = fields->next )
    {
	if( IS_FUNCT(fields->type) )
	{
	    yyerror("struct/union member may not be a function");
	    return 1;
	}
	if( IS_STRUCT(fields->type) &&
			 !strcmp( fields->type->V_STRUCT->tag, cur_struct->tag))
	{
	    yyerror("Recursive struct/union definition\n");
	    return 1;
	}
    }
    return 0;
}
/*----------------------------------------------------------------------*/

int  figure_struct_offsets( p, is_struct )
symbol	*p;				/* Chain of symbols for fields.	*/
int	is_struct;			/* 0 if a union. 		*/
{
    /* Figure the field offsets and return the total structure size. Assume
     * that the first element of the structure is aligned on a worst-case
     * boundary. The returned size is always an even multiple of the worst-case
     * alignment. The offset to each field is put into the "level" field of the
     * associated symbol.
     */

    int	 align_size, obj_size;
    int  offset = 0;

    for( ; p ; p = p->next )
    {
	if( !is_struct )			/* It's a union. */
	{
	    offset   = max( offset, get_sizeof( p->type ) );
	    p->level = 0;
	}
	else
	{
	    obj_size   = get_sizeof    ( p->type );
	    align_size = get_alignment ( p->type );

	    while( offset % align_size )
		++offset;

	    p->level  = offset;
	    offset   += obj_size ;
	}
    }
    /* Return the structure size: the current offset rounded up to the	    */
    /* worst-case alignment boundary. You need to waste space here in case  */
    /* this is an array of structures.					    */

    while( offset % ALIGN_WORST )
   	++offset ;
    return offset;
}
/*----------------------------------------------------------------------*/

int	get_alignment( p )
link	*p;
{
    /* Returns the alignment--the number by which the base address of the object
     * must be an even multiple. This number is the same one that is returned by
     * get_sizeof(), except for structures which are worst-case aligned, and
     * arrays, which are aligned according to the type of the first element.
     */

    int size;

    if( !p )
    {
	yyerror("INTERNAL, get_alignment: NULL pointer\n");
	exit( 1 );
    }
    if( IS_ARRAY( p )		) return get_alignment( p->next );
    if( IS_STRUCT( p )		) return ALIGN_WORST;
    if( size = get_sizeof( p )	) return size;

    yyerror("INTERNAL, get_alignment: Object aligned on zero boundary\n");
    exit( 1 );
    BCC( return 0; )	/* Keep the compiler happy */
}

PUBLIC void do_enum( sym, val )
symbol	*sym;
int	val;
{
    if( conv_sym_to_int_const( sym, val ) )
	addsym( Symbol_tab, sym );
    else
    {
	yyerror( "%s: redefinition", sym->name );
	discard_symbol( sym );
    }
}
/*---------------------------------------------------------------------*/
PUBLIC	int	conv_sym_to_int_const( sym, val )
symbol  *sym;
int	val;
{
    /* Turn an empty symbol into an integer constant by adding a type chain
     * and initializing the v_int field to val. Any existing type chain is
     * destroyed. If a type chain is already in place, return 0 and do
     * nothing, otherwise return 1. This function processes enum's.
     */
    link *lp;

    if( sym->type )
	return 0;
    lp	    	= new_link();
    lp->class   = SPECIFIER;
    lp->NOUN    = INT;
    lp->SCLASS  = CONSTANT;
    lp->V_INT   = val ;
    sym->type   = lp;
    *sym->rname = '\0';
    return 1;
}
void	fix_types_and_discard_syms( sym )
symbol	*sym;
{
    /* Patch up subroutine arguments to match formal declarations.
     *
     * Look up each symbol in the list. If it's in the table at the correct
     * level, replace the type field with the type for the symbol in the list,
     * then discard the redundant symbol structure. All symbols in the input
     * list are discarded after they're processed.
     *
     * Type checking and automatic promotions are done here, too, as follows:
     *		chars  are converted to int.
     *		arrays are converted to pointers.
     *		structures are not permitted.
     *
     * All new objects are converted to autos.
     */

    symbol *existing, *s;

    while( sym )
    {
	if( !( existing = (symbol *)findsym( Symbol_tab,sym->name) )
					|| sym->level != existing->level )
	{
	    yyerror("%s not in argument list\n", sym->name );
	    exit(1);
	}
	else if( !sym->type ||  !sym->etype )
	{
	    yyerror("INTERNAL, fix_types: Missing type specification\n");
	    exit(1);
	}
	else if( IS_STRUCT(sym->type) )
	{
	    yyerror("Structure passing not supported, use a pointer\n");
	    exit(1);
	}
	else if( !IS_CHAR(sym->type) )
	{
	    /* The existing symbol is of the default int type, don't redefine
	     * chars because all chars are promoted to int as part of the call,
	     * so can be represented as an int inside the subroutine itself.
	     */

	    if( IS_ARRAY(sym->type) ) 		/* Make it a pointer to the */
		sym->type->DCL_TYPE = POINTER;  /* first element.	    */

	    sym->etype->SCLASS = AUTO;		/* Make it an automatic var.  */

	    discard_link_chain(existing->type); /* Replace existing type      */
	    existing->type   = sym->type;	/* chain with the current one.*/
	    existing->etype  = sym->etype;
	    sym->type = sym->etype = NULL;	/* Must be NULL for discard_- */
	}					/* symbol() call, below.      */
	s = sym->next;
	discard_symbol( sym );
	sym = s;
    }
}

/*----------------------------------------------------------------------*/

int	figure_param_offsets( sym )
symbol	*sym;
{
    /* Traverse the chain of parameters, figuring the offsets and initializing
     * the real name (in sym->rname) accordingly. Note that the name chain is
     * assembled in reverse order, which is what you want here because the
     * first argument will have been pushed first, and so will have the largest
     * offset. The stack is 32 bits wide, so every legal type of object will
     * require only one stack element. This would not be the case were floats
     * or structure-passing supported. This also takes care of any alignment
     * difficulties.
     *
     * Return the number of 32-bit stack words required for the parameters.
     */

    int	 offset = 8;		/* First parameter is always at BP(fp+8): */
				/* 4 for old fp, 4 for return address.    */

    for(; sym ; sym = sym->next )
    {
	if( IS_STRUCT(sym->type) )
	{
	    yyerror("Structure passing not supported\n");
	    continue;
	}

	sprintf( sym->rname, "fp+%d", offset );
	offset += SWIDTH ;
    }

    /* Return the offset in stack elements, rounded up if necessary.  */

    return( (offset / SWIDTH) + (offset % SWIDTH != 0) );
}

/*----------------------------------------------------------------------*/

void	print_offset_comment( sym, label )
symbol	*sym;
char	*label;
{
    /* Print a comment listing all the local variables.  */

    for(; sym ; sym = sym->next )
	yycode( "\t/* %16s = %-16s [%s] */\n", sym->rname, sym->name, label );
}
