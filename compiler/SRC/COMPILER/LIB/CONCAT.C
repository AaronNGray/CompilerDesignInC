/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdarg.h>
#include <tools/debug.h>	/* VA_LIST definition */

#if (0 MSC(+1))
#pragma loop_opt(off)  /* Can't do loop optimizations (alias problems) */
#endif

UNIX( int concat( size, dst  		   ) )
ANSI( int concat( int size, char *dst, ... ) )
{
    /* This subroutine concatenates an arbitrary number of strings into a single
     * destination array (dst) of size "size." At most size-1 characters are
     * copied. Use it like this:
     *  	char target[SIZE];
     *		concat( SIZE, target, "first ", "second ",..., "last", NULL);
     */

    char     *src;
    va_list  args;
    va_start( args, dst );

    while( (src = va_arg(args,char *)) && size > 1 )
	while( *src && size-- > 1 )
	    *dst++ = *src++ ;

    *dst++ = '\0';
    va_end( args );
    return (size <= 1 && src && *src) ? -1 : size ;
}

  /*----------------------------------------------------------------------*/

#ifdef MAIN
  main()
  {
       char target[ 128 ];
       concat( 128, target, " first ", " second ", " last", NULL);
       printf("target is <%s>\n", target );

       concat( 9, target, "see this", "don't see this", " last", NULL);
       printf("target is <%s>\n", target );
  }
#endif
