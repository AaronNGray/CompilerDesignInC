/*@A (C) 1992 Allen I. Holub                                                */

#include <stdio.h>
#include <stdarg.h>
#include <debug.h>	/* for VA_LIST definition */

/* This file contains two output routines that replace the ones in yydebug.c,
 * found in l.lib and used by occs for output. Link this file to a LeX-
 * generated lexical analyzer when an occs-generated parser is not present.
 * Then use yycomment() for messages to stdout, yyerror() for messages to
 * stderr.
 */

PUBLIC	void yycomment( fmt  VA_LIST )
char	*fmt;
{
    /* Works like printf(). */

    va_list	  args;
    va_start( args, fmt );
    vfprintf( stdout, fmt, args );
    va_end  ( args );
}

PUBLIC void yyerror( fmt  VA_LIST )
char	*fmt;
{
    /* Works like printf() but prints an error message along with the
     * current line number and lexeme.
     */

    va_list	  args;
    va_start( args, fmt );

    fprintf ( stderr, "ERROR on line %d, near <%s>\n", yylineno, yytext );
    vfprintf( stderr, fmt, args );
    va_end  ( args );
}

