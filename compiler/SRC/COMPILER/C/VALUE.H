/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __VALUE_H
#define __VALUE_H
#include "symtab.h"	/* For NAME_MAX definition */

#define VALNAME_MAX (NAME_MAX * 2)     /* Max. length of string in value.name */

typedef struct value
{
    char     name[ VALNAME_MAX ]; /* Operand that accesses the value.	    */
    link     *type;		  /* Variable's type (start of chain).	    */
    link     *etype;		  /* Variable's type (end of chain).	    */
    symbol   *sym;		  /* Original symbol.			    */
    unsigned lvalue   :1;	  /* 1 = lvalue, 0 = rvalue.		    */
    unsigned is_tmp   :1;	  /* 1 if a temporary variable.		    */
    unsigned offset   :14;	  /* Absolute value of offset from base of  */
				  /* temporary region on stack to variable. */
} value;


#define LEFT  1		     /* Second argument to shift_name() in value.c, */
#define RIGHT 0		     /* 			   discussed below. */

#define CONST_STR(p) tconst_str((p)->type)   /* Simplify tconst_str() calls */
					     /* with value arguments by     */
					     /* extracting the type field.  */
#endif /* __VALUE_H */
