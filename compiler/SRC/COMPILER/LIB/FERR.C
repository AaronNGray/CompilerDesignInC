/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <tools/debug.h>
#include <tools/l.h>

/* Note that ferr() is typed as int, even though it usually doesn't return,
 * because it's occasionally useful to use it inside a conditional expression
 * where a type will be required.
 */

ANSI( int  ferr( char *fmt, ... )	)
KnR ( int  ferr( fmt            )	)
KnR ( char *fmt;			)
{
  D(	void		(**ret_addr_p)(); )
	va_list	  	args;
UNIX(   extern int	fputc();     )

	va_start( args, fmt );

ANSI(	if( fmt ) prnt   ( (int(*)(int,...))fputc, stderr, fmt, args ); )
UNIX(	if( fmt ) prnt   ( (int(*)(       ))fputc, stderr, fmt, args ); )
	else	  perror ( va_arg(args, char* )     );

  D(	ret_addr_p  = (void (**)()) &fmt ;				  )
  D(	fprintf(stderr, "\n\t--ferr() Called from %p\n", ret_addr_p[-2]); )

	va_end( args );
	exit( on_ferr() );
	BCC( return 0; ) 	/* Keep the compiler happy */
}
