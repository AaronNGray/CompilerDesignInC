/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include <tools/prnt.h>

/*------------------------------------------------------------------
 * Glue formatting workhorse functions to various environments. One of two
 * versions of the workhorse function is used, depending on various #defines:
 *
 * if ANSI(x) expands to x,  uses vsprintf(), a	standard ANSI function
 * otherwise		  ,  uses _doprnt(),	standard UNIX function
 */

#if (0 ANSI(+1)) /*-----------------------------------------------------*/
#include <stdarg.h>

PUBLIC void prnt(   prnt_t	ofunct,		/* declared in prnt.h */
		    void	*funct_arg,
		    char	*format,
		    va_list	args
		)
{
    char  buf[256], *p ;

    vsprintf(buf, format, args);	/* prototype is in <stdio.h> */
    for( p = buf; *p ; p++ )
	(*ofunct)( *p, funct_arg );
}

PUBLIC void stop_prnt( void ){}

#else /* K&R C ---------------------------------------------------------------*/
#include <varargs.h>

static FILE	*Tmp_file = NULL ;
static char	*Tmp_name ;

PUBLIC void prnt( ofunct, funct_arg, fmt, argp )
int	(*ofunct)();
void	*funct_arg;
char	*fmt;
int	*argp;
{
    int	  c;
    char  *mktemp();

    if( !Tmp_file )
	if( !(Tmp_file = fopen( Tmp_name = mktemp("yyXXXXXX"), "w+") ))
	{
	    fprintf(stderr,"Can't open temporary file %s\n", Tmp_name );
	    exit( 1 );
	}

    _doprnt( fmt, argp, Tmp_file );
    putc   ( 0,		Tmp_file );
    rewind (    	Tmp_file );

    while( (c = getc(Tmp_file)) != EOF && c )
	(*ofunct)( c, funct_arg );
    rewind( Tmp_file );
}

PUBLIC void  stop_prnt()
{
    if( Tmp_file )
    {
	fclose( Tmp_file );		/* Remove prnt temporary file */
	unlink( Tmp_name );
	Tmp_file = NULL;
    }
}

/*----------------------------------------------------------------------*/
#ifdef vfprintf		/* Get rid of the macros in debug.h */
#undef vfprintf
#endif
#ifdef vprintf
#undef vprintf
#endif

PUBLIC void vfprintf( stream, fmt, argp )
FILE *stream;
char *fmt, *argp;
{
    _doprnt( fmt, argp, stream );
}

PUBLIC void vprintf( fmt, argp )
char *fmt, *argp;
{
    _doprnt( fmt, argp, stdout );
}

PRIVATE	void putstr(c, p)
int	c;
char	**p;
{
    *(*p)++ = c ;
}

PUBLIC void vsprintf( str, fmt, argp )
char *str, *fmt, *argp;
{
    prnt( putstr, &str, fmt, argp );
    *str = '\0' ;
}
#endif
