/*@A (C) 1992 Allen I. Holub                                                */
   /* Default routine to print user-supplied portion of the value stack.  */
#include <stdio.h>
#include <tools/debug.h>

#ifdef __TURBOC__
#    pragma argsused	/* Turn off "argument not used" warning */
#endif

char	*yypstk(val,sym)
void	*val;
char	*sym;
{
    static char buf[32];
    sprintf( buf, "%d", *(int *)val );
    return buf;
}
