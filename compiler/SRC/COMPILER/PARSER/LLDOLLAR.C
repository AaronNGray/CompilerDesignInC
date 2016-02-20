/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include "parser.h"

#ifdef __TURBOC__
#pragma argsused
#endif

PUBLIC char *do_dollar( num, rhs_size, lineno, prod, field )
int 	   num;						/* the N in $N */
int 	   rhs_size;					/* not used    */
int 	   lineno;					/* not used    */
PRODUCTION *prod;					/* not used    */
char	   *field;					/* not used    */
{
    static char buf[32];

    if( num == DOLLAR_DOLLAR )
	return "Yy_vsp->left";
    else
    {
	sprintf( buf, "(Yy_vsp[%d].right)",  num );	/* assuming that num */
	return buf;					/* has < 16 digits   */
    }
}
