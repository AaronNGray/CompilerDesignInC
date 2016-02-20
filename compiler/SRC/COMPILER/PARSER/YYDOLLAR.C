/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include "parser.h"

PUBLIC char *do_dollar( num, rhs_size, lineno, prod, fname )
int	    num;	/* The N in $N, DOLLAR_DOLLAR for $$ (DOLLAR_DOLLAR)  */
			/* is defined in parser.h, discussed in Chapter Four. */
int	    rhs_size;	/* Number of symbols on right-hand side, 0 for tail   */
int	    lineno;	/* Input line number for error messages 	      */
PRODUCTION  *prod;	/* Only used if rhs_size is >= 0        	      */
char	    *fname;	/* name in $<name>N				      */
{
    static  char buf[ 128 ];
    int	i, len ;

    if( num == DOLLAR_DOLLAR )					/* Do $$ */
    {
	strcpy( buf, "Yy_val" );

	if( *fname )					        /* $<name>N */
	    sprintf( buf+6, ".%s", fname );

	else if( fields_active() )
	{
	    if( *prod->lhs->field )
		sprintf( buf+6, ".%s", prod->lhs->field );
	    else
	    {
		error( WARNING, "Line %d: No <field> assigned to $$, ", lineno);
		error( NOHDR,   "using default int field\n" );
		sprintf( buf+6, ".%s", DEF_FIELD );
	    }
	}
    }
    else
    {
	if( num < 0 )
	    ++num;

	if( rhs_size < 0 )				     /* $N is in tail */
	    sprintf( buf, "Yy_vsp[ Yy_rhslen-%d ]" , num );
	else
	{
	    if(  (i = rhs_size - num) < 0 )
		error( WARNING, "Line %d: Illegal $%d in production\n",
								lineno, num);
	    else
	    {
		ANSI( len = sprintf( buf, "yyvsp[%d]", i ); )
		KnR ( 	    sprintf( buf, "yyvsp[%d]", i ); )
		KnR ( len = strlen ( buf );		    )

		if( *fname )				        /* $<name>N */
		    sprintf( buf + len, ".%s", fname );

		else if( fields_active() )
		{
		    if( num <= 0 )
		    {
			error(NONFATAL,"Can't use %%union field with negative");
			error(NOHDR,   " attributes. Use $<field>-N\n"        );
		    }
		    else if( * (prod->rhs)[num-1]->field )
		    {
			sprintf( buf + len, ".%s", (prod->rhs)[num-1]->field );
		    }
		    else
		    {
			error( WARNING, "Line %d: No <field> assigned to $%d.",
								lineno, num );
			error( NOHDR,   " Using default int field\n" );
			sprintf( buf + len, ".%s", DEF_FIELD );
		    }
		}
	    }
	}
    }

    return buf;
}
