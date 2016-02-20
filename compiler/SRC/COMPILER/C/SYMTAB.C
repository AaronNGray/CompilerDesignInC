/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/l.h>
#include <tools/compiler.h>
#include <tools/c-code.h>
#include "symtab.h"	   /* Symbol-table definitions.			      */
#include "value.h"	   /* Value definitions.			      */
#include "proto.h"	   /* Prototypes for all functions in this directory. */
#include "label.h"	   /* Labels to use for compiler-generated symbols.   */

#if ( (0 ANSI(+1)) == 0 )  /* If not an ANSI compilers	*/
#define sprintf _sprintf   /* Use ANSI-compatible version out of comp.lib */
#endif
/*----------------------------------------------------------------------*/
PRIVATE symbol	  *Symbol_free = NULL; /* Free-list of recycled symbols.    */
PRIVATE link	  *Link_free   = NULL; /* Free-list of recycled links.	    */
PRIVATE structdef *Struct_free = NULL; /* Free-list of recycled structdefs. */

#define LCHUNK	10	    /* new_link() gets this many nodes at one shot.*/

static  void psym	P((symbol     *sym_p, FILE *fp	));
static  void pstruct	P((structdef *sdef_p, FILE *fp	));
/*----------------------------------------------------------------------*/
PUBLIC	symbol	*new_symbol( name, scope )
char	*name;
int	scope;
{
    symbol *sym_p;

    if( !Symbol_free )					/* Free list is empty.*/
	sym_p = (symbol *) newsym( sizeof(symbol) );
    else						/* Unlink node from   */
    {							/* the free list.     */
	sym_p 	    = Symbol_free;
	Symbol_free = Symbol_free->next ;
	memset( sym_p, 0, sizeof(symbol) );
    }

    strncpy( sym_p->name, name, sizeof(sym_p->name) );
    sym_p->level = scope;
    return sym_p;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
PUBLIC	void	discard_symbol( sym )
symbol	*sym;
{
    /* Discard a single symbol structure and any attached links and args. Note
     * that the args field is recycled for initializers, the process is
     * described later on in the text (see value.c in the code), but you have to
     * test for a different type here. Sorry about the forward reference.
     */

    if( sym )
    {
	if( IS_FUNCT( sym->type ) )
	    discard_symbol_chain( sym->args );	      /* Function arguments. */
	else
	    discard_value( (value *)sym->args );      /* If an initializer.  */

	discard_link_chain( sym->type );	      /* Discard type chain. */

	sym->next   = Symbol_free ;		      /* Put current symbol */
	Symbol_free = sym;			      /* in the free list.  */
    }
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
PUBLIC	void	discard_symbol_chain(sym)   /* Discard an entire cross-linked */
symbol	*sym;				    /* chain of symbols.              */
{
    symbol *p = sym;

    while( sym )
    {
	p = sym->next;
	discard_symbol( sym );
	sym = p;
    }
}
/*----------------------------------------------------------------------*/
PUBLIC link	*new_link( )
{
    /* Return a new link. It's initialized to zeros, so it's a declarator.
     * LCHUNK nodes are allocated from malloc() at one time.
     */

    link *p;
    int   i;

    if( !Link_free )
    {
	if( !(Link_free = (link *) malloc( sizeof(link) * LCHUNK )) )
	{
	    yyerror("INTERNAL, new_link: Out of memory\n");
	    exit( 1 );
	}

	for( p = Link_free, i = LCHUNK; --i > 0; ++p )   /* Executes LCHUNK-1 */
	    p->next = p + 1;			  	 /*	       times. */

	p->next = NULL ;
    }

    p 	       = Link_free;
    Link_free  = Link_free->next;
    memset( p, 0, sizeof(link) );
    return p;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
PUBLIC  void	discard_link_chain( p )
link    *p;
{
    /* Discard all links in the chain. Nothing is removed from the structure
     * table, however. There's no point in discarding the nodes one at a time
     * since they're already linked together, so find the first and last nodes
     * in the input chain and link the whole list directly.
     */

    link *start ;

    if( start = p )
    {
	while( p->next )	/* find last node */
	    p = p->next;

	p->next   = Link_free;
	Link_free = start;
    }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PUBLIC	void	discard_link( p )		 /* Discard a single link. */
link	*p;
{
    p->next    = Link_free;
    Link_free  = p;
}
/*----------------------------------------------------------------------*/
PUBLIC	structdef *new_structdef( tag )		/* Allocate a new structdef. */
char	*tag;
{
    structdef *sdef_p;

    if( !Struct_free )
	sdef_p = (structdef *) newsym( sizeof(structdef) );
    else
    {
	sdef_p 	    = Struct_free;
	Struct_free = (structdef *)(Struct_free->fields);
	memset( sdef_p, 0, sizeof(structdef) );
    }
    strncpy( sdef_p->tag, tag, sizeof(sdef_p->tag) );
    return sdef_p;
}
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
#ifdef STRUCTS_ARE_DISCARDED  /* they aren't in the present compiler */

PUBLIC	void	discard_structdef( sdef_p )
structdef	*sdef_p;
{
    /* Discard a structdef and any attached fields, but don't discard linked
     * structure definitions.
     */

    if( sdef_p )
    {
	discard_symbol_chain( sdef_p->fields );
	sdef_p->fields = (symbol *)Struct_free ;
	Struct_free    = sdef_p;
    }
}
#endif

PUBLIC	void	add_declarator( sym, type )
symbol	*sym;
int	type;
{
    /* Add a declarator link to the end of the chain, the head of which is
     * pointed to by sym->type and the tail of which is pointed to by
     * sym->etype. *head must be NULL when the chain is empty. Both pointers
     * are modified as necessary.
     */

    link *link_p;

    if( type == FUNCTION && IS_ARRAY(sym->etype) )
    {
	yyerror("Array of functions is illegal, assuming function pointer\n");
	add_declarator( sym, POINTER );
    }

    link_p           = new_link();	  /* The default class is DECLARATOR. */
    link_p->DCL_TYPE = type;

    if( !sym->type )
	sym->type = sym->etype = link_p;
    else
    {
	sym->etype->next = link_p;
	sym->etype       = link_p;
    }
}

PUBLIC void spec_cpy( dst, src ) /* Copy all initialized fields in src to dst.*/
link *dst, *src;
{
    if( src->NOUN 	) dst->NOUN	= src->NOUN     ;
    if( src->SCLASS	) dst->SCLASS	= src->SCLASS   ;
    if( src->LONG 	) dst->LONG	= src->LONG     ;
    if( src->UNSIGNED	) dst->UNSIGNED = src->UNSIGNED ;
    if( src->STATIC	) dst->STATIC  	= src->STATIC   ;
    if( src->EXTERN	) dst->EXTERN   = src->EXTERN   ;
    if( src->tdef	) dst->tdef     = src->tdef     ;

    if( src->SCLASS == CONSTANT || src->NOUN == STRUCTURE)
	memcpy( &dst->VALUE, &src->VALUE, sizeof(src->VALUE) );
}

PUBLIC	link  *clone_type( tchain, endp )
link  *tchain;		/* input:  Type chain to duplicate.         	  */
link  **endp;		/* output: Pointer to last node in cloned chain.  */
{
    /* Manufacture a clone of the type chain in the input symbol. Return a
     * pointer to the cloned chain, NULL if there were no declarators to clone.
     * The tdef bit in the copy is always cleared.
     */

    link  *last, *head = NULL;

    for(; tchain ; tchain = tchain->next )
    {
	if( !head )					/* 1st node in chain. */
	    head = last = new_link();
	else						/* Subsequent node.   */
	{
	    last->next = new_link();
	    last       = last->next;
	}

	memcpy( last, tchain, sizeof(*last) );
	last->next = NULL;
	last->tdef = 0;
    }

    *endp = last;
    return head;
}

/*----------------------------------------------------------------------*/

PUBLIC	int	the_same_type( p1, p2, relax )
link	*p1, *p2;
int	relax;
{
    /* Return 1 if the types match, 0 if they don't. Ignore the storage class.
     * If "relax" is true and the array declarator is the first link in the
     * chain, then a pointer is considered equivalent to an array.
     */

     if( relax && IS_PTR_TYPE(p1) && IS_PTR_TYPE(p2) )
     {
	p1 = p1->next;
	p2 = p2->next;
     }

     for(; p1 && p2 ; p1 = p1->next, p2 = p2->next)
     {
	if( p1->class != p2->class )
	    return 0;

	if( p1->class == DECLARATOR )
	{
	    if( (p1->DCL_TYPE != p2->DCL_TYPE) ||
			(p1->DCL_TYPE==ARRAY && (p1->NUM_ELE != p2->NUM_ELE)) )
		return 0;
	}
	else						/* this is done last */
	{
	    if( (p1->NOUN     == p2->NOUN     ) && (p1->LONG == p2->LONG ) &&
	        (p1->UNSIGNED == p2->UNSIGNED ) )
	    {
		return ( p1->NOUN==STRUCTURE ) ? p1->V_STRUCT == p2->V_STRUCT
					       : 1 ;
	    }
	    return 0;
	}
     }

     yyerror("INTERNAL the_same_type: Unknown link class\n");
     return 0;
}

/*----------------------------------------------------------------------*/

PUBLIC	int	get_sizeof( p )
link	*p;
{
    /* Return the size in bytes of an object of the the type pointed to by p.
     * Functions are considered to be pointer sized because that's how they're
     * represented internally.
     */

    int size;

    if( p->class == DECLARATOR )
	size = (p->DCL_TYPE==ARRAY) ? p->NUM_ELE * get_sizeof(p->next) : PSIZE;
    else
    {
	switch( p->NOUN )
	{
	case CHAR:	size = CSIZE;  			break;
	case INT:	size = p->LONG ? LSIZE : ISIZE;	break;
	case STRUCTURE:	size = p->V_STRUCT->size;	break;
	case VOID:	size = 0;  			break;
	case LABEL:	size = 0;  			break;
	}
    }

    return size;
}

/*----------------------------------------------------------------------*/

PUBLIC	symbol	*reverse_links( sym )
symbol	*sym;
{
    /* Go through the cross-linked chain of "symbols", reversing the direction
     * of the cross pointers. Return a pointer to the new head of chain
     * (formerly the end of the chain) or NULL if the chain started out empty.
     */

    symbol *previous, *current, *next;

    if( !sym )
	return NULL;

    previous = sym;
    current  = sym->next;

    while( current )
    {
	next		= current->next;
	current->next	= previous;
	previous	= current;
	current		= next;
    }

    sym->next = NULL;
    return previous;
}

PUBLIC	char	*sclass_str( class )	/* Return a string representing the */
int	class;				/* indicated storage class.	    */
{
    return class==CONSTANT   ? "CON" :
	   class==REGISTER   ? "REG" :
	   class==TYPEDEF    ? "TYP" :
	   class==AUTO       ? "AUT" :
	   class==FIXED      ? "FIX" : "BAD SCLASS" ;
}

/*----------------------------------------------------------------------*/

PUBLIC	char	*oclass_str( class )	/* Return a string representing the */
int	class;				/* indicated output storage class.  */
{
    return class==PUB ? "PUB"  :
	   class==PRI ? "PRI"  :
	   class==COM ? "COM"  :
	   class==EXT ? "EXT"  :  "(NO OCLS)" ;
}

/*----------------------------------------------------------------------*/

PUBLIC	char	*noun_str( noun )	/* Return a string representing the */
int	noun;				/* indicated noun.		    */
{
    return noun==INT	    ? "int"    :
	   noun==CHAR	    ? "char"   :
	   noun==VOID	    ? "void"   :
	   noun==LABEL	    ? "label"  :
	   noun==STRUCTURE  ? "struct" : "BAD NOUN" ;
}

/*----------------------------------------------------------------------*/

PUBLIC char  *attr_str( spec_p )	/* Return a string representing all */
specifier *spec_p;			/* attributes in a specifier other  */
{					/* than the noun and storage class. */
    static char str[5];

    str[0] = ( spec_p->_unsigned ) ? 'u' : '.' ;
    str[1] = ( spec_p->_static   ) ? 's' : '.' ;
    str[2] = ( spec_p->_extern   ) ? 'e' : '.' ;
    str[3] = ( spec_p->_long     ) ? 'l' : '.' ;
    str[4] = '\0';

    return str;
}

/*----------------------------------------------------------------------*/

PUBLIC	char	*type_str ( link_p )
link	*link_p;		       /* Return a string representing the    */
{				       /* type represented by the link chain. */
    int		i;
    static char target [ 80 ];
    static char	buf    [ 64 ];
    int		available  = sizeof(target) - 1;

    *buf    = '\0';
    *target = '\0';

    if( !link_p )
	return "(NULL)";

    if( link_p->tdef )
    {
	strcpy( target, "tdef " );
	available -= 5;
    }

    for(; link_p ; link_p = link_p->next )
    {
	if( IS_DECLARATOR(link_p) )
	{
	    switch( link_p->DCL_TYPE )
	    {
	    case POINTER:    i = sprintf(buf, "*" );			break;
	    case ARRAY:	     i = sprintf(buf, "[%d]", link_p->NUM_ELE);	break;
	    case FUNCTION:   i = sprintf(buf, "()" ); 			break;
	    default: 	     i = sprintf(buf, "BAD DECL" );	        break;
	    }
	}
	else  /* it's a specifier */
	{
	    i = sprintf( buf, "%s %s %s %s",    noun_str  ( link_p->NOUN     ),
					        sclass_str( link_p->SCLASS   ),
					        oclass_str( link_p->OCLASS   ),
					        attr_str  ( &link_p->select.s));

	    if( link_p->NOUN==STRUCTURE  || link_p->SCLASS==CONSTANT  )
	    {
		strncat( target, buf, available );
		available -= i;

		if( link_p->NOUN == STRUCTURE )
		    i = sprintf(buf, " %s", link_p->V_STRUCT->tag ?
					    link_p->V_STRUCT->tag : "untagged");

		else if( IS_INT(link_p)  ) sprintf( buf,"=%d", link_p->V_INT  );
		else if( IS_UINT(link_p) ) sprintf( buf,"=%u", link_p->V_UINT );
		else if( IS_LONG(link_p) ) sprintf( buf,"=%ld",link_p->V_LONG );
		else if( IS_ULONG(link_p)) sprintf( buf,"=%lu",link_p->V_ULONG);
		else			   continue;
	    }
	}

	strncat( target, buf, available );
	available -= i;
    }

    return target;
}

/*----------------------------------------------------------------------*/

PUBLIC char *tconst_str( type )
link	*type;		   	   /* Return a string representing the value  */
{				   /* field at the end of the specified type  */
    static char buf[80];	   /* (which must be char*, char, int, long,  */
				   /* unsigned int, or unsigned long). Return */
    buf[0] = '?';	   	   /* "?" if the type isn't any of these.     */
    buf[1] = '\0';

    if( IS_POINTER(type)  &&  IS_CHAR(type->next) )
    {
	sprintf( buf, "%s%d", L_STRING, type->next->V_INT );
    }
    else if( !(IS_AGGREGATE(type) || IS_FUNCT(type)) )
    {
	switch( type->NOUN )
	{
	case CHAR:	sprintf( buf, "'%s' (%d)", bin_to_ascii(
				    type->UNSIGNED ? type->V_UINT
						     : type->V_INT,1),
				    type->UNSIGNED ? type->V_UINT
						     : type->V_INT,1	);
			break;

	case INT:	if( type->LONG )
			{
			    if( type->UNSIGNED )
				sprintf(buf, "%luL", type->V_ULONG);
			    else
				sprintf(buf, "%ldL", type->V_LONG );
			}
			else
			{
			    if( type->UNSIGNED )
				sprintf( buf, "%u", type->V_UINT);
			    else
				sprintf( buf, "%d", type->V_INT );
			}
			break;
	}
    }

    if( *buf == '?' )
	yyerror("Internal, tconst_str: Can't make constant for type %s\n",
							    type_str( type ));
    return buf;
}

/*----------------------------------------------------------------------*/

PUBLIC	char	*sym_chain_str( chain )
symbol	*chain;
{
    /* Return a string listing the names of all symbols in the input chain (or
     * a constant value if the symbol is a constant). Note that this routine
     * can't call type_str() because the second-order recursion messes up the
     * buffers. Since the routine is used only for occasional diagnostics, it's
     * not worth fixing this problem.
     */

    int		i;
    static char buf[80];
    char	*p	= buf;
    int 	avail	= sizeof( buf ) - 1;

    *buf = '\0';
    while( chain && avail > 0 )
    {
	if( IS_CONSTANT(chain->etype) )
	    i = sprintf( p, "%0.*s", avail - 2, "const" );
	else
	    i = sprintf( p, "%0.*s", avail - 2, chain->name );

	p     += i;
	avail -= i;

	if( chain = chain->next )
	{
	    *p++ = ',' ;
	    i -= 2;
	}
    }

    return buf;
}

/*----------------------------------------------------------------------*/

PRIVATE void psym( sym_p, fp )			/* Print one symbol to fp. */
symbol	*sym_p;
FILE	*fp;
{
    fprintf(fp, "%-18.18s %-18.18s %2d %p %s\n",
				 sym_p->name,
				 sym_p->type ? sym_p->rname : "------",
				 sym_p->level,
				 MSC((void far *)) sym_p->next,
				 type_str( sym_p->type ) );
}

/*----------------------------------------------------------------------*/

PRIVATE	void pstruct( sdef_p, fp )	/* Print a structure definition to fp */
structdef    *sdef_p;			/* including all the fields & types.  */
FILE	     *fp;
{
    symbol	*field_p;

    fprintf(fp, "struct <%s> (level %d, %d bytes):\n",
					sdef_p->tag,sdef_p->level,sdef_p->size);

    for( field_p = sdef_p->fields; field_p; field_p=field_p->next )
    {
	fprintf(fp, "    %-20s (offset %d) %s\n",
		       field_p->name, field_p->level, type_str(field_p->type));
    }
}

/*----------------------------------------------------------------------*/

PUBLIC void print_syms( filename )	/* Print the entire symbol table to   */
char	*filename;			/* the named file. Previous contents  */
{					/* of the file (if any) are destroyed.*/
    FILE *fp;

    if( !(fp = fopen(filename,"w")) )
	yyerror("Can't open symbol-table file\n");
    else
    {
	fprintf(fp, "Attributes in type field are:   upel\n"   );
	fprintf(fp, "    unsigned (. for signed)-----+|||\n"   );
	fprintf(fp, "    private  (. for public)------+||\n"   );
	fprintf(fp, "    extern   (. for common)-------+|\n"   );
	fprintf(fp, "    long     (. for short )--------+\n\n" );

        fprintf(fp,"name               rname              lev   next   type\n");
	ptab( Symbol_tab, (ptab_t)psym, fp, 1 );

	fprintf(fp, "\nStructure table:\n\n");
	ptab( Struct_tab, (ptab_t)pstruct, fp, 1 );

	fclose( fp );
    }
}
