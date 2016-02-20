/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>

#include <tools/stack.h>	/* stack-manipulation macros	*/
#undef  stack_cls		/* Make all stacks static 	*/
#define stack_cls static

#include "parser.h"
#include "llout.h"

/*  ACTS.C	Action routines used by both llama and occs. These build
 *		up the symbol table from the input specification.
 */

void	find_problems	P(( SYMBOL *sym 			)); /* local */
int	c_identifier	P(( char *name				));

/* Prototypes for public functions are in parser.h */
/*----------------------------------------------------------------------*/
extern int	yylineno;		/* Input line number--created by LeX. */

PRIVATE char	Field_name[NAME_MAX];	/* Field name specified in <name>.    */
PRIVATE int	Goal_symbol_is_next =0; /* If true, the next nonterminal is   */
					/* the goal symbol.		      */
#ifdef OCCS
PRIVATE int 	Associativity;		/* Current associativity direction.   */
PRIVATE int     Prec_lev = 0;		/* Precedence level. Incremented      */
					/* after finding %left, etc.,	      */
					/* but before the names are done.     */
PRIVATE int	Fields_active = 0;	/* Fields are used in the input.      */
					/* (If they're not, then automatic    */
					/* field-name generation, as per      */
#endif					/* %union, is not activated.)         */

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * The following stuff (that's a technical term) is used for processing nested
 * optional (or repeating) productions defined with the occs [] and []*
 * operators. A stack is kept and, every time you go down another layer of
 * nesting, the current nonterminal is stacked and an new nonterminal is
 * allocated. The previous state is restored when you're done with a level.
 */

#define SSIZE	8		/* Max. optional-production nesting level */
typedef struct _cur_sym_
{
    char	lhs_name[NAME_MAX]; /* Name associated with left-hand side. */
    SYMBOL	*lhs;		    /* Pointer to symbol-table entry for    */
				    /* 		the current left-hand side. */
    PRODUCTION	*rhs;		    /* Pointer to current production.	    */

} CUR_SYM;


CUR_SYM	Stack[ SSIZE ], 	 /* Stack and 				    */
        *Sp = Stack + (SSIZE-1); /* stack pointer. It's inconvenient to use */
				 /* stack.h because stack is of structures. */
/*======================================================================
 * Support routines for actions
 */
#ifdef __TURBOC__
#pragma argsused
#endif

PUBLIC	void print_tok( stream, format, arg )
FILE	*stream;
char	*format;		/* not used here but supplied by pset() */
int	arg;
{
    /* Print one nonterminal symbol to the specified stream.  */

    if     ( arg == -1      ) fprintf(stream, "null "			);
    else if( arg == -2      ) fprintf(stream, "empty "			);
    else if( arg == _EOI_   ) fprintf(stream, "$ "			);
    else if( arg == EPSILON ) fprintf(stream, "<epsilon> "		);
    else	              fprintf(stream, "%s ", Terms[arg]->name	);
}

/*----------------------------------------------------------------------
 * The following three routines print the symbol table. Pterm(), pact(), and
 * pnonterm() are all called indirectly through ptab(), called in
 * print_symbols(). They print the terminal, action, and nonterminal symbols
 * from the symbol table, respectively.
 */

PUBLIC	void pterm( sym, stream )
SYMBOL  *sym;
FILE	*stream;
{
    OX( int i; )
    if( !ISTERM(sym) )
	return;

#   ifdef LLAMA
	fprintf( stream, "%-16.16s  %3d\n", sym->name, sym->val );
#   else
	fprintf( stream, "%-16.16s  %3d    %2d     %c     <%s>\n",
				sym->name,
				sym->val,
				Precedence[sym->val].level ,
				(i = Precedence[sym->val].assoc) ? i : '-',
				sym->field );
#   endif
}
/*----------------------------------------------------------------------*/
PUBLIC	void pact( sym, stream )
SYMBOL  *sym;
FILE	*stream;
{
    if( !ISACT(sym) )
	    return;

    fprintf( stream, "%-5s %3d,",    sym->name, sym->val );
    fprintf( stream, " line %-3d: ", sym->lineno    );
    fputstr( sym->string, 55, stream );
    fprintf( stream, "\n");
}
/*----------------------------------------------------------------------*/
PUBLIC  char *production_str( prod )
PRODUCTION   *prod;
{
    /* return a string representing the production */

    int    	i, nchars, avail;
    static char buf[80];
    char	*p;

    ANSI( nchars = sprintf(buf,"%s ->", prod->lhs->name ); )
    KnR ( 	   sprintf(buf,"%s ->", prod->lhs->name ); )
    KnR ( nchars = strlen (buf			        ); )

    p      = buf + nchars;
    avail  = sizeof(buf) - nchars - 1;

    if( !prod->rhs_len )
	sprintf(p, " (epsilon)" );
    else
	for( i = 0; i < prod->rhs_len && avail > 0 ; ++i )
	{
	    ANSI( nchars = sprintf(p, " %0.*s", avail-2, prod->rhs[i]->name ); )
	    KnR ( 	   sprintf(p, " %0.*s", avail-2, prod->rhs[i]->name ); )
	    KnR ( nchars = strlen (p					    ); )
	    avail -= nchars;
	    p     += nchars;
	}

    return buf;
}
/*----------------------------------------------------------------------*/
PUBLIC	void pnonterm( sym, stream )
SYMBOL  *sym;
FILE	*stream;
{
    PRODUCTION	*p;
    int    	chars_printed;

    stack_dcl( pstack, PRODUCTION *, MAXPROD );

    if( !ISNONTERM(sym) )
	    return;

    fprintf( stream, "%s (%3d)  %s", sym->name, sym->val,
				sym == Goal_symbol ? "(goal symbol)" : "" );

    OX( fprintf( stream, " <%s>\n", sym->field );	)
    LL( fprintf( stream, "\n" );			)

    if( Symbols > 1 )
    {
	/* Print first and follow sets only if you want really verbose output.*/

	fprintf( stream, "   FIRST : " );
	pset( sym->first, (pset_t)print_tok, stream );

	LL(   fprintf(stream, "\n   FOLLOW: ");		)
	LL(   pset( sym->follow, (pset_t)print_tok, stream );	)

	fprintf(stream, "\n");
    }

    /* Productions are put into the SYMBOL in reverse order because it's easier
     * to tack them on to the beginning of the linked list. It's better to print
     * them in forward order, however, to make the symbol table more readable.
     * Solve this problem by stacking all the productions and then popping
     * elements to print them. Since the pstack has MAXPROD elements, it's not
     * necessary to test for stack overflow on a push.
     */

    for( p = sym->productions ; p ; p = p->next )
	push( pstack, p );

    while( !stack_empty( pstack ) )
    {
	p = pop(pstack);
	chars_printed = fprintf(stream, "   %3d: %s",
						p->num, production_str( p ));

	LL(  for( ; chars_printed <= 45; ++chars_printed )	)
	LL(      putc( '.', stream );				)
	LL(  fprintf(stream, "SELECT: ");			)
	LL(  pset( p->select, (pset_t) print_tok, stream );	)

	OX( if( p->prec )					)
	OX( {							)
	OX(     for( ; chars_printed <= 60; ++chars_printed )	)
	OX(          putc( '.', stream );			)
	OX(	if( p->prec )					)
	OX(         fprintf(stream, "PREC %d", p->prec );	)
	OX( }							)

	putc('\n', stream);
    }

    fprintf(stream, "\n");
}
/*----------------------------------------------------------------------*/
PUBLIC	void print_symbols( stream )
FILE	*stream;
{
    /* Print out the symbol table. Nonterminal symbols come first for the sake
     * of the 's' option in yydebug(); symbols other than production numbers can
     * be entered symbolically. ptab returns 0 if it can't print the symbols
     * sorted (because there's no memory. If this is the case, try again but
     * print the table unsorted).
     */

    putc( '\n', stream );

    D( printf("stream        = _iob[%ld] (0x%x)\n", (long)(stream-_iob),\
    								stream ); )
    D( printf("stream->_ptr  = 0x%x\n", stream->_ptr );			  )
    D( printf("stream->_base = 0x%x\n", stream->_base );		  )
    D( printf("stream->_flag = 0x%x\n", stream->_flag );		  )
    D( printf("stream->_file = 0x%x\n", stream->_file );		  )
    D( printf("stream->_cnt  = %d\n",   stream->_cnt );			  )

    fprintf( stream, "---------------- Symbol table ------------------\n" );
    fprintf( stream, "\nNONTERMINAL SYMBOLS:\n\n" );
    if( ptab( Symtab, (ptab_t)pnonterm, stream, 1 ) == 0 )
    	ptab( Symtab, (ptab_t)pnonterm, stream, 0 );

    fprintf( stream, "\nTERMINAL SYMBOLS:\n\n");
    OX( fprintf( stream, "name             value  prec  assoc   field\n"); )
    LL( fprintf( stream, "name             value\n"); 			   )

    if( ptab( Symtab, (ptab_t)pterm, stream, 1 ) == 0 )
    	ptab( Symtab, (ptab_t)pterm, stream, 0 );

    LL(  fprintf( stream, "\nACTION SYMBOLS:\n\n");			  )
    LL(  if( !ptab( Symtab, (ptab_t)pact, stream, 1 ) )			  )
    LL(       ptab( Symtab, (ptab_t)pact, stream, 0 );			  )
    LL(  fprintf( stream, "----------------------------------------\n" ); )
}
/*----------------------------------------------------------------------
 * Problems() and find_problems work together to find unused symbols and
 * symbols that are used but not defined.
 */

PRIVATE	void find_problems( sym )
SYMBOL  *sym;
{
    if( !sym->used && sym!=Goal_symbol )
	error( WARNING,  "<%s> not used (defined on line %d)\n",
						    sym->name, sym->set );
    if( !sym->set && !ISACT(sym) )
	error( NONFATAL, "<%s> not defined (used on line %d)\n",
						    sym->name, sym->used );
}

PUBLIC	int problems()
{
    /* Find, and print an error message, for all symbols that are used but not
     * defined, and for all symbols that are defined but not used. Return the
     * number of errors after checking.
     */

    ptab( Symtab, (ptab_t)find_problems, NULL, 0 );
    return yynerrs;
}
/*----------------------------------------------------------------------*/
   /* Don't need an explicit hash function because the name is at the
    * top of the structure. Here's what it would look like if you moved
    * the field:
    */
#ifdef NEVER
  PRIVATE int hash_funct( p )
  SYMBOL *p;
  {
      if( !*p->name )
  	lerror( FATAL, "Illegal empty symbol name\n" );

      return hash_add( p->name );
  }
#endif

PUBLIC	void init_acts()
{
    /* Various initializations that can't be done at compile time. Call this
     * routine before starting up the parser. The hash-table size (157) is
     * an arbitrary prime number, roughly the number symbols expected in the
     * table. Note that using hash_pjw knocks about 25% off the execution
     * time with the examples in the book (as compared to hash_add.)
     */

    static SYMBOL   bogus_symbol;

    strcpy( bogus_symbol.name, "End of Input" );
    Terms[0] 	 = &bogus_symbol;

    Symtab	 = maketab( 157, hash_pjw, strcmp );
    LL( Synch  	 = newset(); )
}
/*----------------------------------------------------------------------*/
PUBLIC	SYMBOL *make_term( name )		 /* Make a terminal symbol */
char	*name;
{
    SYMBOL	*p;

    if( !c_identifier(name) )
	lerror(NONFATAL, "Token names must be legitimate C identifiers\n");

    else if( p = (SYMBOL *) findsym(Symtab, name) )
	lerror(WARNING, "Terminal symbol <%s> already declared\n", name );
    else
    {
	if( Cur_term >= MAXTERM )
	    lerror(FATAL, "Too many terminal symbols (%d max.).\n", MAXTERM );

	p = (SYMBOL *) newsym( sizeof(SYMBOL) );
	strncpy ( p->name,  name,       NAME_MAX );
	strncpy ( p->field, Field_name, NAME_MAX );
	addsym  ( Symtab, p );

	p->val = ++Cur_term ;
	p->set = yylineno;

	Terms[Cur_term] = p;
    }
    return p;
}
/*----------------------------------------------------------------------*/
PRIVATE c_identifier( name )		/* Return true only if name is  */
char	*name;				/* a legitimate C identifier.	*/
{
    if( isdigit( *name ) )
	return 0;

    for(; *name ; ++name )
	if( !( isalnum(*name) || *name == '_' ))
	    return 0;

    return 1;
}
/*----------------------------------------------------------------------*/
PUBLIC void	first_sym()
{
    /*  This routine is called just before the first rule following the
     *  %%. It's used to point out the goal symbol;
     */

    Goal_symbol_is_next = 1;
}
/*----------------------------------------------------------------------*/
PUBLIC  SYMBOL	*new_nonterm( name, is_lhs )
char	*name;
int	is_lhs;
{
    /* Create, and initialize, a new nonterminal. is_lhs is used to
     * differentiate between implicit and explicit declarations. It's 0 if the
     * nonterminal is added because it was found on a right-hand side. It's 1 if
     * the nonterminal is on a left-hand side.
     *
     * Return a pointer to the new symbol or NULL if an attempt is made to use a
     * terminal symbol on a left-hand side.
     */

    SYMBOL	*p;

    if( p = (SYMBOL *) findsym( Symtab, name ) )
    {
	if( !ISNONTERM( p ) )
	{
	    lerror(NONFATAL, "Symbol on left-hand side must be nonterminal\n");
	    p = NULL;
	}
    }
    else if( Cur_nonterm >= MAXNONTERM )
    {
	lerror(FATAL, "Too many nonterminal symbols (%d max.).\n", MAXTERM );
    }
    else 			       /* Add new nonterminal to symbol table */
    {
	p = (SYMBOL *) newsym( sizeof(SYMBOL) );
	strncpy ( p->name,  name,	NAME_MAX );
	strncpy ( p->field, Field_name,	NAME_MAX );

	p->val = ++Cur_nonterm ;
	Terms[Cur_nonterm] = p;

	addsym  ( Symtab, p );
    }

    if( p )			            /* (re)initialize new nonterminal */
    {
	if( Goal_symbol_is_next )
	{
	    Goal_symbol = p;
	    Goal_symbol_is_next = 0;
	}

	if( !p->first )
	    p->first  = newset();

LL(	if( !p->follow )		)
LL(	    p->follow = newset();	)

	p->lineno = yylineno ;

	if( is_lhs )
	{
	    strncpy( Sp->lhs_name, name, NAME_MAX );
	    Sp->lhs      = p ;
	    Sp->rhs      = NULL;
	    Sp->lhs->set = yylineno;
	}
    }

    return p;
}
/*----------------------------------------------------------------------*/
PUBLIC	void new_rhs()
{
    /* Get a new PRODUCTION and link it to the head of the production chain.
     * of the current nonterminal. Note that the start production MUST be
     * production 0. As a consequence, the first rhs associated with the first
     * nonterminal MUST be the start production. Num_productions is initialized
     * to 0 when it's declared.
     */

    PRODUCTION	*p;

    if( !(p = (PRODUCTION *) calloc(1, sizeof(PRODUCTION))) )
	lerror(FATAL, "No memory for new right-hand side\n");

    p->next	         = Sp->lhs->productions;
    Sp->lhs->productions = p;

    LL( p->select = newset(); )

    if( (p->num = Num_productions++) >= MAXPROD )
	lerror(FATAL, "Too many productions (%d max.)\n", MAXPROD );

    p->lhs  = Sp->lhs;
    Sp->rhs = p;
}
/*----------------------------------------------------------------------*/
PUBLIC	void add_to_rhs( object, is_an_action )
char	*object;
int	is_an_action;	/* 0 of not an action, line number otherwise */
{
    SYMBOL	*p;
    int	i;
    char	buf[32];

    /* Add a new element to the RHS currently at top of stack. First deal with
     * forward references. If the item isn't in the table, add it. Note that,
     * since terminal symbols must be declared with a %term directive, forward
     * references always refer to nonterminals or action items. When we exit the
     * if statement, p points at the symbol table entry for the current object.
     */

    if( !(p = (SYMBOL *) findsym( Symtab, object)) )  /* not in tab yet */
    {
	if( !is_an_action )
	{
	    if( !(p = new_nonterm( object, 0 )) )
	    {
		 /* Won't get here unless p is a terminal symbol */

		lerror(FATAL, "(internal) Unexpected terminal symbol\n");
		return;
	    }
	}
	else
	{
	    /* Add an action. All actions are named "{DDD}" where DDD is the
	     * action number. The curly brace in the name guarantees that this
	     * name won't conflict with a normal name. I am assuming that calloc
	     * is used to allocate memory for the new node (ie. that it's
	     * initialized to zeros).
	     */

	    sprintf(buf, "{%d}", ++Cur_act - MINACT );

	    p = (SYMBOL *) newsym( sizeof(SYMBOL) );
	    strncpy ( p->name, buf, NAME_MAX );
	    addsym  ( Symtab, p );

	    p->val    = Cur_act      ;
	    p->lineno = is_an_action ;

	    if( !(p->string = strdup(object)) )
		lerror(FATAL, "Insufficient memory to save action\n");
	}
    }

    p->used = yylineno;

    if( (i = Sp->rhs->rhs_len++)  >=  MAXRHS )
	lerror(NONFATAL, "Right-hand side too long (%d max)\n", MAXRHS );
    else
    {
	LL(  if( i == 0  &&  p == Sp->lhs )				    )
	LL(	lerror(NONFATAL, "Illegal left recursion in production.\n");)
       	OX(  if( ISTERM( p ) )						    )
	OX(	Sp->rhs->prec = Precedence[ p->val ].level ;		    )

	Sp->rhs->rhs[ i     ] = p;
	Sp->rhs->rhs[ i + 1 ] = NULL;		/* NULL terminate the array. */

	if( !ISACT(p) )
	    ++( Sp->rhs->non_acts );
    }
}
/*----------------------------------------------------------------------
 * The next two subroutines handle repeating or optional subexpressions. The
 * following mappings are done, depending on the operator:
 *
 * S : A [B]  C ;	 S   -> A 001 C
 *			 001 -> B | epsilon
 *
 * S : A [B]* C ;	 S   -> A 001 C				(occs)
 *			 001 -> 001 B | epsilon
 *
 * S : A [B]* C ;	 S   -> A 001 C				(llama)
 *			 001 -> B 001 | epsilon
 *
 * In all situations, the right hand side that we've collected so far is
 * pushed and a new right-hand side is started for the subexpression. Note that
 * the first character of the created rhs name (001 in the previous examples)
 * is a space, which is illegal in a user-supplied production name so we don't
 * have to worry about conflicts. Subsequent symbols are added to this new
 * right-hand side. When the ), ], or *) is found, we finish the new right-hand
 * side, pop the stack and add the name of the new right-hand side to the
 * previously collected left-hand side.
 */

#ifdef __TURBOC__
#pragma argsused
#endif

PUBLIC	void start_opt( lex )	/* Start an optional subexpression    */
char	*lex;			/* not used at present, but might be. */
{
	char   name[32];
	static int num = 0;

	--Sp;				      /* Push current stack element   */
	sprintf( name, " %06d", num++);       /* Make name for new production */
	new_nonterm( name, 1 );	      	      /* Create a nonterminal for it. */
	new_rhs();			      /* Create epsilon production.   */
	new_rhs();			      /* and production for sub-prod. */
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC  void end_opt( lex )			/* end optional subexpression */
char	*lex;
{
        char    *name = Sp->lhs_name ;
    OX( int     i;	)
    OX( SYMBOL  *p;	)

    if( lex[1] == '*'  )		   /* Process a [...]* 		      */
    {
	add_to_rhs( name, 0 );	       	   /* Add right-recursive reference.  */

#ifdef OCCS				   /* If occs, must be left recursive.*/
	i = Sp->rhs->rhs_len - 1;	   /* Shuffle things around.          */
	p = Sp->rhs->rhs[ i ];
	memmove( &(Sp->rhs->rhs)[1], &(Sp->rhs->rhs)[0],
					i * sizeof( (Sp->rhs->rhs)[1] ) );
	Sp->rhs->rhs[ 0 ] = p ;
#endif
    }

    ++Sp;				      /* discard top-of-stack element */
    add_to_rhs( name, 0 );
}
/*======================================================================
 * The following routines have alternate versions, one set for llama and another
 * for occs. The routines corresponding to features that aren't supported in one
 * or the other of these programs print error messages.
 */

#ifdef LLAMA

PUBLIC	void add_synch( name )
char	*name;
{
    /*  Add "name" to the set of synchronization tokens
    */

    SYMBOL	*p;

    if( !(p = (SYMBOL *) findsym( Symtab, name )) )
	lerror(NONFATAL,"%%synch: undeclared symbol <%s>.\n", name );

    else if( !ISTERM(p) )
	lerror(NONFATAL,"%%synch: <%s> not a terminal symbol\n", name );

    else
	ADD( Synch, p->val );
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC void new_lev( how )
int how;
{
    switch( how )
    {
    case  0 : /* initialization: ignore it */				 break;
    case 'l': lerror (NONFATAL, "%%left not recognized by LLAMA\n"    ); break;
    case 'r': lerror (NONFATAL, "%%right not recognized by LLAMA\n"   ); break;
    default : lerror (NONFATAL, "%%nonassoc not recognized by LLAMA\n"); break;
    }
}

PUBLIC void prec( name )
char *name;
{
    lerror( NONFATAL, "%%prec %s not recognized by LLAMA\n", name );
}

#ifdef __TURBOC__
#pragma argsused
#endif

PUBLIC void union_def( action )
char	*action;
{
    lerror(NONFATAL,"%%union not recognized by LLAMA\n");
}

#ifdef __TURBOC__
#pragma argsused
#endif

PUBLIC void prec_list( name ) char *name;
{
}

PUBLIC void new_field( field_name )
char	*field_name;
{
    if( *field_name )
	lerror(NONFATAL, "<name> not supported by LLAMA\n");
}

#else  /*============================================================*/

#ifdef __TURBOC__
#pragma argsused
#endif

PUBLIC void add_synch(yytext)
char *yytext;
{
    lerror(NONFATAL, "%%synch not supported by OCCS\n");
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC void new_lev( how )
int how;
{
    /* Increment the current precedence level and modify "Associativity"
     * to remember if we're going left, right, or neither.
     */

    if( Associativity = how )	/*  'l', 'r', 'n', (0 if unspecified)  */
	++Prec_lev;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC void prec_list( name )
char	*name;
{
    /* Add current name (in yytext) to the precision list. "Associativity" is
     * set to 'l', 'r', or 'n', depending on whether we're doing a %left,
     * %right, or %nonassoc. Also make a nonterminal if it doesn't exist
     * already.
     */

    SYMBOL *sym;

    if( !(sym = (SYMBOL *) findsym(Symtab,name)) )
	sym = make_term( name );

    if( !ISTERM(sym) )
	lerror(NONFATAL, "%%left or %%right, %s must be a token\n", name );
    else
    {
	Precedence[ sym->val ].level = Prec_lev ;
	Precedence[ sym->val ].assoc = Associativity  ;
    }
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC void prec( name )
char *name;
{
    /* Change the precedence level for the current right-hand side, using
     * (1) an explicit number if one is specified, or (2) an element from the
     * Precedence[] table otherwise.
     */

    SYMBOL	*sym;

    if( isdigit(*name) )
	Sp->rhs->prec = atoi(name);				       /* (1) */
    else
    {
	if( !(sym = (SYMBOL *) findsym(Symtab,name)) )
	    lerror(NONFATAL, "%s (used in %%prec) undefined\n" );

	else if( !ISTERM(sym) )
	    lerror(NONFATAL, "%s (used in %%prec) must be terminal symbol\n" );

	else
	    Sp->rhs->prec = Precedence[ sym->val ].level;	       /* (2) */
    }
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC void union_def( action )
char	*action;
{
    /*  create a YYSTYPE definition for the union, using the fields specified
     *  in the %union directive, and also appending a default integer-sized
     *  field for those situation where no field is attached to the current
     *  symbol.
     */

    while( *action && *action != '{' )	/* Get rid of everything up to the */
	++action;			/* open brace 			   */
    if( *action )			/* and the brace itself		   */
	++action;

    output( "typedef union\n" );
    output( "{\n" );
    output( "    int   %s;  /* Default field, used when no %%type found */",
								DEF_FIELD );
    output( "%s\n",  action );
    output( "yystype;\n\n"  );
    output( "#define YYSTYPE yystype\n" );
    Fields_active = 1;
}

ANSI( PUBLIC	int fields_active( void )	)
KnR ( PUBLIC	int fields_active(      )	)
{
    return Fields_active ;		/* previous %union was specified */
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC  void new_field( field_name )
char	*field_name;
{
    /* Change the name of the current <field> */

    char	*p;

    if( !*field_name )
	*Field_name = '\0' ;
    else
    {
	if( p = strchr(++field_name, '>') )
	    *p = '\0' ;

	strncpy( Field_name, field_name, sizeof(Field_name) );
    }
}
#endif
