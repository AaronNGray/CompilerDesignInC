/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __SYMTAB_H
#define __SYMTAB_H

#include <tools/debug.h>
#include <tools/hash.h>

#ifdef ALLOC			      /* Allocate variables if ALLOC defined. */
#   define ALLOC_CLS /* empty */

#else
#   define ALLOC_CLS   extern
#endif

#define NAME_MAX  32			    /* Maximum identifier length.     */
#define LABEL_MAX 32			    /* Maximum output-label length.   */

typedef struct symbol			    /* Symbol-table entry.	      */
{
    char   name  [NAME_MAX+1];		    /* Input variable name.	      */
    char   rname [NAME_MAX+1];		    /* Actual variable name.	      */

    unsigned	    level     : 13 ;	    /* Declaration lev., field offset.*/
    unsigned	    implicit  : 1  ;	    /* Declaration created implicitly.*/
    unsigned	    duplicate : 1  ;	    /* Duplicate declaration.	      */

    struct   link   *type;		    /* First link in declarator chain.*/
    struct   link   *etype;		    /* Last  link in declarator chain.*/
    struct   symbol *args;    		    /* If a funct decl, the arg list. */
					    /* If a var, the initializer.     */
    struct   symbol *next;		    /* Cross link to next variable at */
					    /* current nesting level.	      */
} symbol;


ALLOC_CLS  HASH_TAB  *Symbol_tab;	    /* The actual table. */

#define POINTER		0		/* Values for declarator.type. 	  */
#define ARRAY		1
#define FUNCTION	2

typedef struct declarator
{
    int dcl_type;			/* POINTER, ARRAY, or FUNCTION	  */
    int num_ele;			/* If class==ARRAY, # of elements */
} declarator;


#define INT	  0		/* specifier.noun. INT has the value 0 so   */
#define CHAR	  1		/* that an uninitialized structure defaults */
#define VOID	  2		/* to int, same goes for EXTERN, below.	    */
#define STRUCTURE 3
#define LABEL	  4
				/* specifier.sclass			*/
#define FIXED	  0		/*     At a fixed address.		*/
#define REGISTER  1		/*     In a register.			*/
#define AUTO	  2		/*     On the run-time stack.		*/
#define TYPEDEF	  3		/*     Typedef.				*/
#define CONSTANT  4		/*     This is a constant.		*/

				/* Output (C-code) storage class	*/
#define NO_OCLASS 0		/*	No output class (var is auto).  */
#define PUB	  1		/*	public				*/
#define PRI	  2		/*	private				*/
#define EXT	  3		/*	extern				*/
#define COM	  4		/*	common				*/


typedef struct specifier
{
    unsigned noun      :3;    /* CHAR INT STRUCTURE LABEL          	 */
    unsigned sclass    :3;    /* REGISTER AUTO FIXED CONSTANT TYPEDEF  	 */
    unsigned oclass    :3;    /* Output storage class: PUB PRI COM EXT.  */
    unsigned _long     :1;    /* 1=long.      0=short.		  	 */
    unsigned _unsigned :1;    /* 1=unsigned.  0=signed.	  		 */
    unsigned _static   :1;    /* 1=static keyword found in declarations. */
    unsigned _extern   :1;    /* 1=extern keyword found in declarations. */

    union
    {				/* Value if constant:			  */
	int	       v_int;	/* Int & char values. If a string const., */
				/* is numeric component of the label.	  */
	unsigned int   v_uint;  /* Unsigned int constant value.		  */
	long	       v_long;  /* Signed long constant value.		  */
	unsigned long  v_ulong; /* Unsigned long constant value.	  */

	struct structdef *v_struct; /* If this is a struct, points at a	*/
				    /* structure-table element.		*/
    } const_val;


} specifier;


#define DECLARATOR	0
#define SPECIFIER	1

typedef struct link
{
    unsigned class   : 1;		/* DECLARATOR or SPECIFIER 	      */
    unsigned tdef    : 1;	        /* For typedefs. If set, current link */
					/* chain was created by a typedef.    */
    union
    {
	specifier     s;		/* If class == DECLARATOR	      */
	declarator    d;		/* If class == SPECIFIER	      */
    }
    select ;
    struct link  *next;		       /* Next element of chain.	      */

} link;


/*----------------------------------------------------------------------
 * Use the following p->XXX where p is a pointer to a link structure.
 */

#define NOUN		select.s.noun
#define SCLASS		select.s.sclass
#define LONG		select.s._long
#define UNSIGNED	select.s._unsigned
#define EXTERN		select.s._extern
#define STATIC		select.s._static
#define OCLASS		select.s.oclass

#define DCL_TYPE	select.d.dcl_type
#define NUM_ELE		select.d.num_ele

#define VALUE		select.s.const_val
#define V_INT		VALUE.v_int
#define V_UINT		VALUE.v_uint
#define V_LONG		VALUE.v_long
#define V_ULONG		VALUE.v_ulong
#define V_STRUCT	VALUE.v_struct

/*--------------------------------------------------------------------*/
/* Use the following XXX(p) where p is a pointer to a link structure. */

#define IS_SPECIFIER(p)  ((p) && (p)->class==SPECIFIER )
#define IS_DECLARATOR(p) ((p) && (p)->class==DECLARATOR )
#define IS_ARRAY(p)   ((p) && (p)->class==DECLARATOR && (p)->DCL_TYPE==ARRAY   )
#define IS_POINTER(p) ((p) && (p)->class==DECLARATOR && (p)->DCL_TYPE==POINTER )
#define IS_FUNCT(p)   ((p) && (p)->class==DECLARATOR && (p)->DCL_TYPE==FUNCTION)
#define IS_STRUCT(p)  ((p) && (p)->class==SPECIFIER  && (p)->NOUN == STRUCTURE )
#define IS_LABEL(p)   ((p) && (p)->class==SPECIFIER  && (p)->NOUN == LABEL     )

#define IS_CHAR(p)      ((p) && (p)->class == SPECIFIER && (p)->NOUN == CHAR )
#define IS_INT(p)       ((p) && (p)->class == SPECIFIER && (p)->NOUN == INT  )
#define IS_UINT(p)	( 	IS_INT(p) && (p)->UNSIGNED	 	 )
#define IS_LONG(p)      ( 	IS_INT(p) && (p)->LONG 		 	 )
#define IS_ULONG(p)	( 	IS_INT(p) && (p)->LONG && (p)->UNSIGNED	 )
#define IS_UNSIGNED(p)	((p) && (p)->UNSIGNED				 )

#define IS_AGGREGATE(p)	 ( IS_ARRAY(p) || IS_STRUCT(p)    )
#define IS_PTR_TYPE(p)	 ( IS_ARRAY(p) || IS_POINTER(p)   )

#define	IS_CONSTANT(p)     (IS_SPECIFIER(p) && (p)->SCLASS == CONSTANT	)
#define	IS_TYPEDEF(p)      (IS_SPECIFIER(p) && (p)->SCLASS == TYPEDEF	)
#define	IS_INT_CONSTANT(p) (IS_CONSTANT(p)  && (p)->NOUN   == INT	)
typedef struct structdef
{
    char          tag[NAME_MAX+1];  /* Tag part of structure definition.      */
    unsigned char level;	    /* Nesting level at which struct declared.*/
    symbol        *fields;	    /* Linked list of field declarations.     */
    unsigned      size;		    /* Size of the structure in bytes.	      */

} structdef;


ALLOC_CLS HASH_TAB  *Struct_tab;   /* The actual table.		*/
#define CSIZE	BYTE_WIDTH	/* char */
#define CTYPE	"byte"

#define ISIZE	WORD_WIDTH	/* int */
#define ITYPE	"word"

#define LSIZE	LWORD_WIDTH	/* long */
#define LTYPE	"lword"

#define PSIZE	PTR_WIDTH	/* pointer: 32-bit (8086 large model) */
#define PTYPE	"ptr"

#define STYPE	"record"	/* structure, size undefined */
#define ATYPE	"array"		/* array,     size undefined */

#endif /* __SYMTAB_H */
