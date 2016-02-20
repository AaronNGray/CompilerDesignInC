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
#include "label.h"

/* OP.C 	This file contains support subroutines for the arithmetic */
/*		operations in c.y.					  */

symbol	*Undecl = NULL; /* When an undeclared symbol is used in an expression,
			 * it is added to the symbol table to suppress subse-
			 * quent error messages. This is a pointer to the head
			 * of a linked list of such undeclared symbols. It is
			 * purged by purge_undecl() at the end of compound
			 * statements, at which time error messages are also
			 * generated.
			 */

static  int	make_types_match P((value **v1p, value **v2p		));
static  int	do_binary_const	 P((value **v1p, int op, value **v2p	));
static  symbol *find_field	 P((structdef *s, char *field_name	));
static  char   *access_with	 P((value *val				));
static  void	dst_opt		 P((value **lp, value **rp, int	commut  ));
/*----------------------------------------------------------------------*/
value	*do_name( yytext, sym )
char	*yytext;		/* Lexeme				    */
symbol	*sym;			/* Symbol-table entry for id, NULL if none. */
{
    link   *chain_end, *lp ;
    value  *synth;
    char   buf[ VALNAME_MAX ];

    /* This routine usually returns a logical lvalue for the referenced symbol.
     * The symbol's type chain is copied into the value and value->name is a
     * string, which when used as an operand, evaluates to the address of the
     * object. Exceptions are aggregate types (arrays and structures), which
     * generate pointer temporary variables, initialize the temporaries to point
     * at the first element of the aggregate, and return an rvalue that
     * references that pointer. The type chain is still just copied from the
     * source symbol, so a structure or pointer must be interpreted as a pointer
     * to the first element/field by the other code-generation subroutines.
     * It's also important that the indirection processing (* [] . ->) set up
     * the same sort of object when the referenced object is an aggregate.
     *
     * Note that !sym must be checked twice, below. The problem is the same one
     * we had earlier with typedefs. A statement like foo(){int x;x=1;} fails
     * because the second x is read when the semicolon is shifted---before
     * putting x into the symbol table. You can't use the trick used earlier
     * because def_list is used all over the place, so the symbol table entries
     * can't be made until local_defs->def_list is processed. The simplest
     * solution is just to call findsym() if NULL was returned from the scanner.
     *
     * The second if(!sym) is needed when the symbol really isn't there.
     */

    if( !sym )
	sym = (symbol *) findsym( Symbol_tab, yytext );

    if( !sym )
	sym  = make_implicit_declaration( yytext, &Undecl );

    if( IS_CONSTANT(sym->etype) )		     /* it's an enum member */
    {
	if( IS_INT(sym->type) )
	    synth = make_icon( NULL, sym->type->V_INT );
	else
	{
	    yyerror("Unexpected noninteger constant\n");
	    synth = make_icon( NULL, 0 );
	}
    }
    else
    {
	gen_comment("%s", sym->name);	/* Next instruction will have symbol */
					/* name as a comment.		     */

	if( !(lp = clone_type( sym->type, &chain_end)) )
	{
	    yyerror("INTERNAL do_name: Bad type chain\n" );
	    synth = make_icon( NULL, 0 );
	}
	else if( IS_AGGREGATE(sym->type) )
	{
	    /* Manufacture pointer to first element */

	    sprintf(buf, "&%s(%s)",
			    IS_ARRAY(sym->type) ? get_prefix(lp) : BYTE_PREFIX,
			    sym->rname );

	    synth = tmp_create(sym->type, 0);
	    gen( "=", synth->name, buf );
	}
	else
	{
	    synth	  = new_value();
	    synth->lvalue = 1	;
	    synth->type	  = lp	;
	    synth->etype  = chain_end;
	    synth->sym	  = sym	;

	    if( sym->implicit || IS_FUNCT(lp) )
		strcpy( synth->name, sym->rname );
	    else
		sprintf(synth->name,
			  (chain_end->SCLASS == FIXED) ? "&%s(&%s)" : "&%s(%s)",
						    get_prefix(lp), sym->rname);
	}
    }

    return synth;
}
/*----------------------------------------------------------------------*/
symbol	*make_implicit_declaration( name, undeclp )
char	*name;
symbol  **undeclp;
{
    /* Create a symbol for the name, put it into the symbol table, and add it
     * to the linked list pointed to by *undeclp. The symbol is an int. The
     * level field is used for the line number.
     */

    symbol	*sym;
    link   	*lp;
    extern int	yylineno;	/* created by LeX */
    extern char *yytext;

    lp		  = new_link();
    lp->class     = SPECIFIER;
    lp->NOUN      = INT;
    sym		  = new_symbol( name, 0 );
    sym->implicit = 1;
    sym->type	  = sym->etype = lp;
    sym->level    = yylineno;	   /* Use the line number for the declaration */
				   /* level so that you can get at it later   */
				   /* if an error message is printed.	      */

    sprintf( sym->rname, "_%1.*s", sizeof(sym->rname)-2, yytext );
    addsym ( Symbol_tab,	   sym		);

    sym->next  = *undeclp;			/* Link into undeclared list. */
    *undeclp   = sym;

    return sym;
}
/*----------------------------------------------------------------------*/
PUBLIC	void purge_undecl()
{
    /* Go through the undeclared list. If something is a function, leave it in
     * the symbol table as an implicit declaration, otherwise print an error
     * message and remove it from the table. This routine is called at the
     * end of every subroutine.
     */

    symbol *sym, *cur ;

    for( sym = Undecl; sym; )
    {
	cur	  = sym;		/* remove current symbol from list */
	sym	  = sym->next;
	cur->next = NULL;

	if( cur->implicit )
	{
	    yyerror("%s (used on line %d) undeclared\n", cur->name, cur->level);
	    delsym( Symbol_tab, cur );
	    discard_symbol( cur );
	}
    }
    Undecl = NULL;
}
value *do_unop( op, val )
int   op;
value *val;
{
    char   *op_buf = "=?" ;
    int	   i;

    if( op != '!' )	/* ~ or unary - */
    {
	if( !IS_CHAR(val->type) && !IS_INT(val->type) )
	    yyerror( "Unary operator requires integral argument\n" );

	else if( IS_UNSIGNED(val->type) && op == '-' )
	    yyerror( "Minus has no meaning on an unsigned operand\n" );

	else if( IS_CONSTANT( val->type ) )
	    do_unary_const( op, val );
	else
	{
	    op_buf[1] = op;
	    gen( op_buf, val->name, val->name );
	}
    }
    else		/* ! */
    {
	if( IS_AGGREGATE( val->type ) )
	    yyerror("May not apply ! operator to aggregate type\n");

	else if( IS_INT_CONSTANT( val->type ) )
	    do_unary_const( '!', val );
	else
	{
	    gen( "EQ",       rvalue(val), "0"	         ); /* EQ(x, 0)       */
	    gen( "goto%s%d", L_TRUE,      i = tf_label() ); /*   goto T000;   */
	    val = gen_false_true( i, val );		    /* fall thru to F */
	}
    }
    return val;
}
/*----------------------------------------------------------------------*/
void do_unary_const( op, val )
int	op;
value	*val;
{
    /* Handle unary constants by modifying the constant's internal value
     * according to the incomming operator. No code is generated.
     */

    link *t = val->type;

    if( IS_INT(t) )
    {
	switch( op )
	{
	case '~':	t->V_INT = ~t->V_INT;	break;
	case '-':	t->V_INT = -t->V_INT;	break;
	case '!':	t->V_INT = !t->V_INT;	break;
	}
    }
    else if( IS_LONG(t) )
    {
	switch( op )
	{
	case '~':	t->V_LONG = ~t->V_LONG;	break;
	case '-':	t->V_LONG = -t->V_LONG;	break;
	case '!':	t->V_LONG = !t->V_LONG;	break;
	}
    }
    else
	yyerror("INTERNAL do_unary_const: unexpected type\n");
}
/*----------------------------------------------------------------------*/
int	tf_label()
{		 	/* Return the numeric component of a label for use as */
    static int label;	/* a true/false target for one of the statements      */
    return ++label;	/* processed by gen_false_true().		      */
}
/*----------------------------------------------------------------------*/

value	*gen_false_true( labelnum, val )
int	labelnum;
value	*val;
{
    /* Generate code to assign true or false to the temporary represented by
     * val. Also, create a temporary to hold the result, using val if it's
     * already the correct type. Return a pointer to the target. The FALSE
     * code is at the top. If val is NULL, create a temporary. Labelnum must
     * have been returned from a previous tf_label() call.
     */

    if( !val )
	val = tmp_create( NULL, 0 );

    else if( !val->is_tmp || !IS_INT(val->type) )
    {
	release_value( val );
	val = tmp_create( NULL, 0 );
    }
    gen( ":%s%d",    L_FALSE,   labelnum );	/* F000:	 	*/
    gen( "=",        val->name, "0" );		/*	  t0 = 0	*/
    gen( "goto%s%d", L_END,	labelnum );	/*	  goto E000;	*/
    gen( ":%s%d",    L_TRUE,    labelnum );	/* T000:	 	*/
    gen( "=",	     val->name, "1" );		/*	  t0 = 1;	*/
    gen( ":%s%d",    L_END,     labelnum );	/* E000:	 	*/
    return val;
}

value	*incop( is_preincrement, op, val )		         /* ++ or -- */
int	is_preincrement;	     /* Is preincrement or predecrement.     */
int	op;			     /* '-' for decrement, '+' for increment */
value	*val;			     /* lvalue to modify.		     */
{
    char  *name;
    char  *out_op  = (op == '+') ? "+=%s%d" : "-=%s%d" ;
    int   inc_amt  ;

    /* You must use rvalue_name() in the following code because rvalue()
     * modifies the val itself--the name field might change. Here, you must use
     * the same name both to create the temporary and do the increment so you
     * can't let the name be modified.
     */

    if( !val->lvalue )
	yyerror("%c%c: lvalue required\n", op, op );
    else
    {
	inc_amt = (IS_POINTER(val->type)) ? get_sizeof(val->type->next) : 1 ;
	name    = rvalue_name( val );

	if( is_preincrement )
	{
	    gen( out_op, name, inc_amt );
	    val = tmp_gen( val->type, val );
	}
	else						 /* Postincrement. */
	{
	    val = tmp_gen( val->type, val );
	    gen( out_op, name, inc_amt );
	}
    }
    return val;
}
value *addr_of( val )
value *val;
{
    /* Process the & operator. Since the incoming value already holds the
     * desired address, all you need do is change the type (add an explicit
     * pointer at the far left) and change it into an rvalue. The first argument
     * is returned.
     */

    link  *p;

    if( val->lvalue )
    {
	p	    = new_link();
	p->DCL_TYPE = POINTER;
	p->next     = val->type;
	val->type   = p;
	val->lvalue = 0;
    }
    else if( !IS_AGGREGATE(val->type) )
	yyerror( "(&) lvalue required\n" );

    return val;
}

value *indirect( offset, ptr )
value *offset;			/* Offset factor (NULL if it's a pointer). */
value *ptr;			/* Pointer that holds base address.	   */
{
    /* Do the indirection, If no offset is given, we're doing a *, otherwise
     * we're doing ptr[offset]. Note that, strictly speaking, you should create
     * the following dumb code:
     *
     *			t0 = rvalue( ptr );	(if ptr isn't a temporary)
     *			t0 += offset		(if doing [offset])
     *			t1 = *t0;		(creates a rvalue)
     *						lvalue attribute = &t1
     *
     * but the first instruction is necessary only if ptr is not already a
     * temporary, the second only if we're processing square brackets.
     *
     * The last two operations cancel if the input is a pointer to a pointer. In
     * this case all you need to do is remove one * declarator from the type
     * chain. Otherwise, you have to create a temporary to take care of the type
     * conversion.
     */

    link  *tmp     ;
    value *synth   ;
    int	  objsize  ;	/* Size of object pointed to (in bytes)		   */

    if( !IS_PTR_TYPE(ptr->type) )
	yyerror( "Operand for * or [N] Must be pointer type\n" );

    rvalue( ptr );   	  /* Convert to rvalue internally. The "name" field   */
			  /* is modified by removing leading &'s from logical */
			  /* lvalues.					      */
    if( offset )	  /* Process an array offset.			      */
    {
	if( !IS_INT(offset->type)  &&  !IS_CHAR(offset->type) )
	    yyerror( "Array index must be an integral type\n" );

	objsize = get_sizeof( ptr->type->next );      /* Size of dereferenced */
						      /* object.	      */

	if( !ptr->is_tmp )			      /* Generate a physical  */
	    ptr = tmp_gen( ptr->type, ptr );	      /* lvalue.              */

	if( IS_CONSTANT( offset->type ) )	      /* Offset is a constant.*/
	{
	    gen("+=%s%d", ptr->name, offset->type->V_INT * objsize );
	}
	else					      /* Offset is not a con- */
	{					      /* stant. Do the arith- */
						      /* metic at run time.   */

	    if( objsize != 1 )			      /* Multiply offset by   */
	    {					      /* size of one object.  */
		if( !offset->is_tmp )
		    offset = tmp_gen( offset->type, offset );

		gen( "*=%s%d", offset->name, objsize );
	    }

	    gen( "+=",  ptr->name, offset->name );    /* Add offset to base. */
	}

	release_value( offset );
    }

    /* The temporary just generated (or the input variable if no temporary
     * was generated) now holds the address of the desired cell. This command
     * must generate an lvalue unless the object pointed to is an aggregate,
     * whereupon it's n rvalue. In any event, you can just label the current
     * cell as an lvalue or rvalue as appropriate and continue. The type must
     * be advanced one notch to compensate for the indirection, however.
     */

    synth	= ptr;
    tmp	  	= ptr->type;			/* Advance type one notch. */
    ptr->type	= ptr->type->next;
    discard_link(tmp);

    if( !IS_AGGREGATE(ptr->type->next) )
	synth->lvalue = 1;		        /* Convert to lvalue.  */

    return synth;
}

value *do_struct( val, op, field_name )
value	*val;
int	op;		/* . or - (the last is for ->)  */
char	*field_name;
{
    symbol	*field;
    link	*lp;

    /* Structure names generate rvalues of type structure. The associated     */
    /* name evaluates to the structure's address, however. Pointers generate  */
    /* lvalues, but are otherwise the same.				      */

    if( IS_POINTER(val->type) )
    {
	if( op != '-' )
	{
	    yyerror("Object to left of -> must be a pointer\n");
	    return val;
	}
	lp	  = val->type;		/* Remove POINTER declarator from  */
	val->type = val->type->next;	/* the type chain and discard it.  */
	discard_link( lp );
    }

    if( !IS_STRUCT(val->type) )
    {
	yyerror("Not a structure.\n");
	return val;
    }				/* Look up the field in the structure table: */

    if( !(field = find_field(val->type->V_STRUCT, field_name)) )
    {
	yyerror("%s not a field\n", field_name );
	return val;
    }

    if( val->lvalue || !val->is_tmp )		/* Generate temporary for     */
	val = tmp_gen( val->type, val );	/* base address if necessary; */
						/* then add the offset to the */
    if( field->level > 0 )			/* desired field.	      */
	gen( "+=%s%d", val->name, field->level );

    if( !IS_AGGREGATE(field->type) )		/* If referenced object isn't */
	val->lvalue = 1;			/* an aggregate, use lvalue.  */

						/* Replace value's type chain */
						/* with type chain for the    */
						/* referenced object:	      */
    lp = val->type;
    if( !(val->type = clone_type( field->type, &val->etype)) )
    {
	yyerror("INTERNAL do_struct: Bad type chain\n" );
	exit(1);
    }
    discard_link_chain( lp );
    access_with( val );	     			/* Change the value's name    */
						/* field to access an object  */
    return val;					/* of the new type.           */
}

/*----------------------------------------------------------------------*/

PRIVATE  symbol *find_field( s, field_name )
structdef	*s;
char		*field_name;
{
    /* Search for "field_name" in the linked list of fields for the input
     * structdef. Return a pointer to the associated "symbol" if the field
     * is there, otherwise return NULL.
     */

    symbol	*sym;
    for( sym = s->fields; sym; sym = sym->next )
    {
	if( !strcmp(field_name, sym->name) )
	    return sym;
    }
    return NULL;
}

/*----------------------------------------------------------------------*/

PRIVATE char	*access_with( val )
value	*val;
{
    /* Modifies the name string in val so that it references the current type.
     * Returns a pointer to the modified string. Only the type part of the
     * name is changed. For example, if the input name is "WP(fp+4)", and the
     * type chain is for an int, the name is be changed to "W(fp+4). If val is
     * an lvalue, prefix an ampersand to the name as well.
     */

    char *p, buf[ VALNAME_MAX ] ;

    strcpy( buf, val->name );
    for( p = buf; *p && *p != '(' /*)*/ ; ++p )		/* find name */
	;

    if( !*p )
	yyerror( "INTERNAL, access_with: missing parenthesis\n" );
    else
	sprintf( val->name, "%s%s%s", val->lvalue ? "&" : "",
						      get_prefix(val->type), p);
    return val->name;
}


PUBLIC value *call( val, nargs )
value *val;
int   nargs;
{
    link   *lp;
    value  *synth;	/* synthesized attribute		*/

    /* The incoming attribute is an lvalue for a function if
     * 		funct()
     * or
     *		int (*p)() = funct;
     *		(*p)();
     *
     * is processed. It's a pointer to a function if p() is used directly.
     * In the case of a logical lvalue (with a leading &), the name will be a
     * function name, and the rvalue can be generated in the normal way by
     * removing the &. In the case of a physical lvalue the name of a variable
     * that holds the function's address is given. No star may be added.
     * If val is an rvalue, then it will never have a leading &.
     */

    if( val->sym  &&  val->sym->implicit  &&  !IS_FUNCT(val->type) )
    {
	/* Implicit symbols are not declared. This must be an implicit function
	 * declaration, so pretend that it's explicit. You have to modify both
	 * the value structure and the original symbol because the type in the
	 * value structure is a copy of the original. Once the modification is
	 * made, the implicit bit can be turned off.
	 */

	lp	    	   = new_link();
	lp->DCL_TYPE 	   = FUNCTION;
	lp->next    	   = val->type;
	val->type   	   = lp;

	lp	    	   = new_link();
	lp->DCL_TYPE 	   = FUNCTION;
	lp->next    	   = val->sym->type;
	val->sym->type 	   = lp;

	val->sym->implicit = 0;
	val->sym->level    = 0;

	yydata( "extern\t%s();\n", val->sym->rname );
    }

    if( !IS_FUNCT(val->type) )
    {
	yyerror( "%s not a function\n", val->name );
	synth = val;
    }
    else
    {
	lp    =  val->type->next;		/* return-value type */
	synth = tmp_create( lp, 0);

	gen( "call", *val->name == '&' ? &val->name[1] : val->name );
	gen(  "=",   synth->name, ret_reg(lp) );

	if( nargs )
	    gen( "+=%s%d" , "sp", nargs * SWIDTH );  /* sp is a byte pointer, */
						     /* so must translate to  */
	release_value( val );			     /* words with "* SWIDTH" */
    }

    return synth;
}

/*----------------------------------------------------------------------*/

char	*ret_reg( p )
link	*p;
{
    /* Return a string representing the register used for a return value of
     * the given type.
     */

    if( IS_DECLARATOR( p ) )
	return "rF.pp";
    else
	switch( p->NOUN )
	{
	case INT:	return (p->LONG) ?  "rF.l" : "rF.w.low" ;
	case CHAR:	return "rF.w.low" ;
	default:	yyerror("INTERNAL ERROR: ret_reg, bad noun\n");
			return "AAAAAAAAAAAAAAAGH!";
	}
}

value *assignment( op, dst, src )
int   op;
value *dst, *src;
{
    char   *src_name;
    char   op_str[8], *p = op_str ;

    if( !dst->lvalue		 ) yyerror    ( "(=) lvalue required\n" );
    if( !dst->is_tmp && dst->sym ) gen_comment( "%s", dst->sym->name	);

    /* Assemble the operator string for gen(). A leading @ is translated by
     * gen() to a * at the far left of the output string. For example,
     * ("@=",x,y) is output as "*x = y".
     */

    if(  *dst->name != '&'	) *p++ =  '@' ;
    if(  op  			) *p++ =  op  ;
    if(  op == '<' || op == '>' ) *p++ =  op  ;		/* <<= or >>= */
    /*  do always        */	  *p++ =  '=' ;
			   	  *p++ = '\0' ;

    src_name = rvalue( src );

    if( IS_POINTER(dst->type) && IS_PTR_TYPE(src->type) )
    {
	if( op )
	    yyerror("Illegal operation (%c= on two pointers);\n", op);

	else if( !the_same_type( dst->type->next, src->type->next, 0) )
	    yyerror("Illegal pointer assignment (type mismatch)\n");

	else
	    gen( "=", dst->name + (*dst->name=='&' ? 1 : 0), src_name );
    }
    else
    {
	/* If the destination type is larger than the source type, perform an
	 * implicit cast (create a temporary of the correct type, otherwise
	 * just copy into the destination. convert_type() releases the source
	 * value.
	 */

	if( !the_same_type( dst->type, src->type, 1) )
	{
	    gen( op_str, dst->name + (*dst->name == '&' ? 1 : 0),
					     convert_type( dst->type, src ) );
	}
	else
	{
	    gen( op_str, dst->name + (*dst->name == '&' ? 1 : 0), src_name );
	    release_value( src );
	}
    }
    return dst;
}

void	or( val, label )
value	*val;
int	label;
{
    val = gen_rvalue( val );

    gen	( "NE",       val->name, "0" 	);
    gen	( "goto%s%d", L_TRUE,	 label	);
    release_value( val );
}
/*----------------------------------------------------------------------*/
value *gen_rvalue( val )
value *val;
{
    /* This function is like rvalue(), except that emits code to generate a
     * physical rvalue from a physical lvalue (instead of just messing with the
     * name). It returns the 'value' structure for the new rvalue rather than a
     * string.
     */

    if( !val->lvalue  || *(val->name) == '&' )  /* rvalue or logical lvalue */
	rvalue( val );				/* just change the name     */
    else
	val = tmp_gen( val->type, val );	/* actually do indirection  */

    return val;
}
void	and( val, label )
value	*val;
int	label;
{
    val = gen_rvalue( val );

    gen	( "EQ",       val->name, "0" 	);
    gen	( "goto%s%d", L_FALSE,	 label	);
    release_value( val );
}
value	*relop( v1, op, v2 )
value	*v1;
int	op;
value	*v2;
{
    char  *str_op ;
    value *tmp;
    int   label;

    v1 = gen_rvalue( v1 );
    v2 = gen_rvalue( v2 );

    if( !make_types_match( &v1, &v2 ) )
	yyerror( "Illegal comparison of dissimilar types\n" );
    else
    {
	switch( op )
	{
	case '>': /* >  */    str_op = "GT";   break;
	case '<': /* <  */    str_op = "LT";   break;
	case 'G': /* >= */    str_op = "GE";   break;
	case 'L': /* <= */    str_op = "LE";   break;
	case '!': /* != */    str_op = "NE";   break;
	case '=': /* == */    str_op = "EQ";   break;
	default:
	    yyerror("INTERNAL, relop(): Bad request: %c\n", op );
	    goto abort;
	}

	gen	( str_op,     v1->name,	v2->name		);
	gen	( "goto%s%d", L_TRUE,	label = tf_label()	);

	if( !(v1->is_tmp && IS_INT( v1->type )) )
	{
	    tmp = v1;			/* try to make v1 an int temporary */
	    v1  = v2;
	    v2  = tmp;
	}
	v1 = gen_false_true( label, v1 );
    }

abort:
    release_value( v2 );			/* discard the other value */
    return v1;
}

/*----------------------------------------------------------------------*/

PRIVATE int make_types_match( v1p, v2p )
value	**v1p, **v2p;
{
    /* Takes care of type conversion. If the types are the same, do nothing;
     * otherwise, apply the standard type-conversion rules to the smaller
     * of the two operands. Return 1 on success or if the objects started out
     * the same type. Return 0 (and don't do any conversions) if either operand
     * is a pointer and the operands aren't the same type.
     */

    value *v1 = *v1p;
    value *v2 = *v2p;

    link  *t1 = v1->type;
    link  *t2 = v2->type;

    if( the_same_type(t1, t2, 0)  &&  !IS_CHAR(t1) )
	return 1;

    if( IS_POINTER(t1) || IS_POINTER(t2) )
	return 0;

    if( IS_CHAR(t1) ) { v1 = tmp_gen(t1, v1);  t1 = v1->type; }
    if( IS_CHAR(t2) ) { v2 = tmp_gen(t2, v2);  t2 = v2->type; }

    if( IS_ULONG(t1) && !IS_ULONG(t2) )
    {
	if( IS_LONG(t2) )
	    v2->type->UNSIGNED = 1;
	else
	    v2 = tmp_gen( t1, v2 );
    }
    else if( !IS_ULONG(t1) && IS_ULONG(t2) )
    {
	if( IS_LONG(t1) )
	    v1->type->UNSIGNED = 1;
	else
	    v1 = tmp_gen( v2->type, v1 );
    }
    else if(  IS_LONG(t1) && !IS_LONG(t2) ) v2 = tmp_gen (t1, v2);
    else if( !IS_LONG(t1) &&  IS_LONG(t2) ) v1 = tmp_gen (t2, v1);
    else if(  IS_UINT(t1) && !IS_UINT(t2) ) v2->type->UNSIGNED = 1;
    else if( !IS_UINT(t1) &&  IS_UINT(t2) ) v1->type->UNSIGNED = 1;

    /* else they're both normal ints, do nothing */

    *v1p = v1;
    *v2p = v2;
    return 1;
}
value	*binary_op( v1, op, v2 )
value	*v1;
int	op;
value	*v2;
{
    char *str_op ;
    int  commutative = 0;	/* operator is commutative */

    if( do_binary_const( &v1, op, &v2 ) )
    {
	release_value( v2 );
	return v1;
    }

    v1 = gen_rvalue( v1 );
    v2 = gen_rvalue( v2 );

    if( !make_types_match( &v1, &v2 ) )
	yyerror("%c%c: Illegal type conversion\n",
					(op=='>' || op =='<') ? op : ' ', op);
    else
    {
	switch( op )
	{
	case '*':
	case '&':
	case '|':
	case '^':   commutative = 1;
	case '/':
	case '%':
	case '<':						/* << */
	case '>':						/* >> */
		    dst_opt( &v1, &v2, commutative );

		    if( op == '<' )
			str_op = "<<=" ;
		    else if( op == '>' )
			str_op = IS_UNSIGNED(v1->type) ? ">L=" : ">>=" ;
		    else
		    {
			str_op = "X=";
			*str_op = op ;
		    }

		    gen( str_op, v1->name, v2->name );
		    break;
	}
    }
    release_value( v2 );
    return v1;
}

/*----------------------------------------------------------------------*/

PRIVATE int	do_binary_const( v1p, op, v2p )
value	**v1p;
int	op;
value	**v2p;
{
    /* If both operands are constants, do the arithmetic. On exit, *v1p
     * is modified to point at the longer of the two incoming types
     * and the result will be in the last link of *v1p's type chain.
     */

    long  x;
    link  *t1 = (*v1p)->type ;
    link  *t2 = (*v2p)->type ;
    value *tmp;

    /* Note that this code assumes that all fields in the union start at the
     * same address.
     */

    if( IS_CONSTANT(t1) && IS_CONSTANT(t2) )
    {
	if( IS_INT(t1) && IS_INT(t2) )
	{
	    switch( op )
	    {
	    case '+':	t1->V_INT +=  t2->V_INT;	break;
	    case '-':	t1->V_INT -=  t2->V_INT;	break;
	    case '*':	t1->V_INT *=  t2->V_INT;	break;
	    case '&':	t1->V_INT &=  t2->V_INT;	break;
	    case '|':	t1->V_INT |=  t2->V_INT;	break;
	    case '^':	t1->V_INT ^=  t2->V_INT;	break;
	    case '/':	t1->V_INT /=  t2->V_INT;	break;
	    case '%':	t1->V_INT %=  t2->V_INT;	break;
	    case '<':	t1->V_INT <<= t2->V_INT;	break;

	    case '>':	if( IS_UNSIGNED(t1) ) t1->V_UINT >>= t2->V_INT;

			else		      t1->V_INT  >>= t2->V_INT;

			break;
	    }
	    return 1;
	}
	else if( IS_LONG(t1) && IS_LONG(t2) )
	{
	    switch( op )
	    {
	    case '+':	t1->V_LONG +=  t2->V_LONG;	break;
	    case '-':	t1->V_LONG -=  t2->V_LONG;	break;
	    case '*':	t1->V_LONG *=  t2->V_LONG;	break;
	    case '&':	t1->V_LONG &=  t2->V_LONG;	break;
	    case '|':	t1->V_LONG |=  t2->V_LONG;	break;
	    case '^':	t1->V_LONG ^=  t2->V_LONG;	break;
	    case '/':	t1->V_LONG /=  t2->V_LONG;	break;
	    case '%':	t1->V_LONG %=  t2->V_LONG;	break;
	    case '<':	t1->V_LONG <<= t2->V_LONG;	break;

	    case '>':	if( IS_UNSIGNED(t1) ) t1->V_ULONG >>= t2->V_LONG;
			else		      t1->V_LONG  >>= t2->V_LONG;
			break;
	    }
	    return 1;
	}
	else if( IS_LONG(t1) && IS_INT(t2) )
	{
	    switch( op )
	    {
	    case '+':	t1->V_LONG +=  t2->V_INT;	break;
	    case '-':	t1->V_LONG -=  t2->V_INT;	break;
	    case '*':	t1->V_LONG *=  t2->V_INT;	break;
	    case '&':	t1->V_LONG &=  t2->V_INT;	break;
	    case '|':	t1->V_LONG |=  t2->V_INT;	break;
	    case '^':	t1->V_LONG ^=  t2->V_INT;	break;
	    case '/':	t1->V_LONG /=  t2->V_INT;	break;
	    case '%':	t1->V_LONG %=  t2->V_INT;	break;
	    case '<':	t1->V_LONG <<= t2->V_INT;	break;

	    case '>':	if( IS_UNSIGNED(t1) ) t1->V_ULONG >>= t2->V_INT;
			else		      t1->V_LONG  >>= t2->V_INT;
			break;
	    }
	    return 1;
	}
	else if( IS_INT(t1) && IS_LONG(t2) )
	{
	    /* Avoid commutativity problems by doing the arithmetic first,
	     * then swapping the operand values.
	     */

	    switch( op )
	    {

	    case '+':	x = t1->V_INT +  t2->V_LONG;

	    case '-':	x = t1->V_INT -  t2->V_LONG;

	    case '*':	x = t1->V_INT *  t2->V_LONG;

	    case '&':	x = t1->V_INT &  t2->V_LONG;

	    case '|':	x = t1->V_INT |  t2->V_LONG;

	    case '^':	x = t1->V_INT ^  t2->V_LONG;

	    case '/':	x = t1->V_INT /  t2->V_LONG;

	    case '%':	x = t1->V_INT %  t2->V_LONG;

	    case '<':	x = t1->V_INT << t2->V_LONG;

	    case '>':	if( IS_UINT(t1) ) x = t1->V_UINT >> t2->V_LONG;

			else              x = t1->V_INT  >> t2->V_LONG;
			break;
	    }

	    t2->V_LONG = x;	/* Modify v1 to point at the larger   */
	    tmp  = *v1p ;	/* operand by swapping *v1p and *v2p. */
	    *v1p = *v2p ;
	    *v2p = tmp  ;
	    return 1;
	}
    }
    return 0;
}
/*----------------------------------------------------------------------*/

PRIVATE void	dst_opt( leftp, rightp, commutative )
value	**leftp;
value	**rightp;
int	commutative;
{
    /* Optimizes various sources and destination as follows:
     *
     * operation is not commutative:
     *		if *left is a temporary:  do nothing
     *		else:   		   create a temporary and
     *					   initialize it to *left,
     *					   freeing *left
     *					   *left = new temporary
     * operation is commutative:
     *		if *left is a temporary	   do nothing
     *	   else if *right is a temporary   swap *left and *right
     *     else			           precede as if commutative.
     */

    value  *tmp;

    if( ! (*leftp)->is_tmp )
    {
	if( commutative && (*rightp)->is_tmp )
	{
	    tmp     = *leftp;
	    *leftp  = *rightp;
	    *rightp = tmp;
	}
	else
	    *leftp = tmp_gen( (*leftp)->type, *leftp );
    }
}
value	*plus_minus( v1, op, v2 )
value	*v1;
int	op;
value	*v2;
{
    value *tmp;
    int   v1_is_ptr;
    int   v2_is_ptr;
    char  *scratch;
    char  *gen_op;

    gen_op    = (op == '+') ? "+=" : "-=";
    v1	      = gen_rvalue( v1 );
    v2	      = gen_rvalue( v2 );
    v2_is_ptr = IS_POINTER(v2->type);
    v1_is_ptr = IS_POINTER(v1->type);

    /* First, get all the error checking out of the way and return if
     * an error is detected.
     */

    if( v1_is_ptr && v2_is_ptr )
    {
	if( op == '+' || !the_same_type(v1->type, v2->type, 1) )
	{
	    yyerror( "Illegal types (%c)\n", op );
	    release_value( v2 );
	    return v1;
	}
    }
    else if( !v1_is_ptr && v2_is_ptr )
    {
	yyerror( "%c: left operand must be pointer", op );
	release_value( v1 );
	return v2;
    }

    /* Now do the work. At this point one of the following cases exist:
     *
     *     v1:    op:   v2:
     *    number [+-] number
     *     ptr   [+-] number
     *     ptr    -    ptr		(types must match)
     */

    if( !(v1_is_ptr || v2_is_ptr) )		/* normal arithmetic */
    {
	if( !do_binary_const( &v1, op, &v2 ) )
	{
	    make_types_match( &v1, &v2 );
	    dst_opt( &v1, &v2, op == '+' );
	    gen( gen_op, v1->name, v2->name );
	}
	release_value( v2 );
	return v1;
    }
    else
    {
	if( v1_is_ptr && v2_is_ptr )		 /* ptr-ptr */
	{
	    if( !v1->is_tmp )
		v1 = tmp_gen( v1->type, v1 );

	    gen( gen_op, v1->name, v2->name );

	    if( IS_AGGREGATE(  v1->type->next ) )
		gen( "/=%s%d", v1->name, get_sizeof(v1->type->next) );
	}
	else if( !IS_AGGREGATE( v1->type->next ) )
	{
					   /* ptr_to_nonaggregate [+-] number */
	    if( !v1->is_tmp )
		v1 = tmp_gen( v1->type, v1 );

	    gen( gen_op, v1->name, v2->name );
	}
	else				    /* ptr_to_aggregate [+-] number */
	{				    /* do pointer arithmetic        */

	    scratch = IS_LONG(v2->type) ? "r0.l" : "r0.w.low" ;

	    gen( "=",      "r1.pp", v1->name			);
	    gen( "=",      scratch, v2->name			);
	    gen( "*=%s%d", scratch, get_sizeof(v1->type->next)	);
	    gen( gen_op,   "r1.pp", scratch			);

	    if( !v1->is_tmp )
	    {
		tmp = tmp_create( v1->type, 0 );
		release_value( v1 );
		v1 = tmp;
	    }

	    gen( "=", v1->name, "r1.pp" );
	}
    }
    release_value( v2 );
    return v1;
}
rlabel( incr )			/* Return the numeric component of the next */
int incr;			/* return label, postincrementing it by one */
{				/* if incr is true.			    */
    static int num;
    return incr ? num++ : num;
}
