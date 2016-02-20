/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>			/* Includes errno def. */
#include <ctype.h>
#include <stdarg.h>			/* For vprintf() */
#include <tools/debug.h>		/* includes VA_LIST definition */
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "nfa.h"
#include "dfa.h"

#define ALLOC
#include "globals.h"

#if (0 MSC(+1) && !defined(_WIN32))
   	    /* This pragma puts a copyright message into the executable file */
   #pragma comment(exestr,"(C)" __DATE__ "Allen I. Holub. All rights reserved.")
#endif
/*----------------------------------------------------------------------*/
PRIVATE  void cmd_line_error P(( int usage, char *fmt, ...	));
PRIVATE  void do_file	     P(( void 				));
PRIVATE  void head	     P(( int suppress_output 		));
PRIVATE  void tail	     P(( void 				));
PRIVATE  void strip_comments P((char *string			));
/*----------------------------------------------------------------------*/
#define DTRAN_NAME  "Yy_nxt" 	/* Name used for DFA transition table. Up to */
				/* 3 characters are appended to the end of   */
				/* this name in the row-compressed tables.   */

#define	E(x)   fprintf(stderr,"%s\n", x)

PRIVATE int	Column_compress = 1;  /* Variables for command-line switches */
PRIVATE int	No_compression  = 0;
PRIVATE int	Threshold	= 4;
PRIVATE int	No_header	= 0;
PRIVATE int	Header_only	= 0;

extern int	Verbose;		/* in globals.h	*/
extern int	No_lines;		/* in globals.h	*/
extern int	Lineno;		     	/* In globals.h, the line number   */
				     	/* used to print #line directives. */
/*----------------------------------------------------------------------*/
ANSI(	PRIVATE	void	cmd_line_error( int usage, char *fmt, ... )	)
KnR (	PRIVATE	void	cmd_line_error(     usage,       fmt      )	)
KnR (	int	usage;							)
KnR (	char	*fmt;							)
{
    /* Print an error message and exit to the operating system. This routine is
     * used much like printf(), except that it has an extra first argument.
     * If "usage" is 0, an error message associated with the current value of
     * errno is printed after printing the format/arg string in the normal way.
     * If "usage" is nonzero, a list of legal command-line switches is printed.
     *
     * I'm using the ANSI conventions for a subroutine with a variable number of
     * arguments. These differ a little from the Unix V conventions. Look up
     * _doprnt() [it's in the "printf(3S)" entry] in the Unix Programmer's
     * Manual for details. A version of the <stdarg.h> macros used here is in
     * Appendix A.
     */

    va_list	args;

    va_start(args, fmt);
    fprintf ( stderr, "LeX: "	);
    vfprintf( stderr, fmt, args	);

    if( !usage )
	perror("");
    else
    {
	E("\n\nUsage is:     LeX [options] file"			);
	E("-f  for (f)ast. Don't compress tables."			);
 	E("-h  suppress (h)eader comment that describes state machine."	);
	E("-H  print the (H)eader only."				);
	E("-l  Suppress #(l)ine directives in the output."		);
	E("-mS Use string S as template name rather than lex.par."	);
	E("-cN use pair (c)ompression, N = threshold (default 4)."	);
	E("-t  Send output to standard output instead of lexyy.c"	);
	E("-u  UNIX mode, newline is \\n, not \\n or \\r"		);
	E("-v  (v)erbose mode, print statistics."			);
	E("-V  More (V)erbose, print internal diagnostics as lex runs."	);
    }
    exit(1);
    va_end(args);
}
/*----------------------------------------------------------------------*/
ANSI(	PUBLIC  void	lerror( int status, char *fmt, ... )	)
KnR (	PUBLIC  void	lerror(     status,       fmt      )	)
KnR (	int	status;						)
KnR (	char	*fmt;						)
{
    /* Print an error message and input line number. Exit with
     * indicated status if "status" is nonzero.
     */

    va_list	args;

    va_start(args, fmt);
    fprintf  ( stderr, "lex, input line %d: ", Actual_lineno );
    vfprintf ( stderr, fmt, args );
    if( status )
	exit( status );
    va_end(args);
}
/*----------------------------------------------------------------------*/
PUBLIC	void main( argc, argv )
int	argc;
char	**argv;
{
    static char 	*p ;
    static int	use_stdout = 0;

    signon();

    for( ++argv, --argc; argc && *(p = *argv) == '-'; ++argv, --argc )
    {
	while( *++p )
	{
	    switch( *p )
	    {
	    case 'f':  No_compression	= 1; 	break;
	    case 'h':  No_header	= 1;	break;
	    case 'H':  Header_only	= 1;	break;
	    case 'l':  No_lines	   	= 1;	break;
	    case 'm':  Template     = p + 1;	goto out;
	    case 'p':  Public	   	= 1;	break;
	    case 't':  use_stdout	= 1; 	break;
	    case 'u':  Unix	   	= 1; 	break;
	    case 'v':  Verbose	   	= 1; 	break;
	    case 'V':  Verbose	   	= 2;	break;
	    case 'c':  Column_compress = 0;

		       if( !isdigit(p[1]) )
			    Threshold = 4;
		       else
		       {
			    Threshold = atoi( ++p );
			    while( *p && isdigit( p[1] ) )
				++p;
		       }
		       break;

	    default:   cmd_line_error(1, "-%c illegal argument.", *p);
		       break;
	    }
	}
    out:;
    }

    if( argc > 1 )
	cmd_line_error( 1, "Too many arguments. Only one file name permitted" );

    else if( argc <= 0 )
    {
	cmd_line_error( 1, "File name required" );
       /* No_lines = 1; */
    }
    else /* argc == 1 */
    {
	if( Ifile = fopen(*argv,"r") )
	    Input_file_name = *argv;
	else
	    cmd_line_error( 0, "Can't open input file %s", *argv );
    }

    if( !use_stdout )
	if( !(Ofile = fopen( Header_only ? "lexyy.h" : "lexyy.c", "w")))
	    cmd_line_error( 0, "Can't open output file lexyy.[ch]" );

    do_file ();
    fclose  ( Ofile );
    fclose  ( Ifile );
    exit    ( 0 );
}
/*----------------------------------------------------------------------*/
PRIVATE void do_file( )
{
    int	   nstates;		/* Number of DFA states	  	*/
    ROW	   *dtran;		/* Transition table	  	*/
    ACCEPT *accept;		/* Set of accept states in dfa	*/
    int	   i;

    /* Process the input file */

    head( Header_only );		/* print everything up to first %% */

    nstates = min_dfa( get_expr, &dtran, &accept );	    /* make DFA */
    if( Verbose )
    {
	printf("%d out of %d DFA states in minimized machine\n", nstates,
								    DFA_MAX );
	printf("%d bytes required for minimized tables\n\n",
		 nstates * MAX_CHARS * sizeof(TTYPE)		/* dtran  */
	       + nstates *             sizeof(TTYPE) );		/* accept */
    }

    if( !No_header )
	pheader( Ofile, dtran, nstates, accept );	    /* print header */
							    /* comment.	    */
    if( !Header_only )
    {
							    /* first part */
							    /* of driver. */
        if( !driver_1( Ofile, !No_lines, Template ) )
	{
	    perror( Template );
	    exit(1);
	}

	if( No_compression )		  		/* compressed tables */
	{
	    fprintf (Ofile ,"YYPRIVATE YY_TTYPE  %s[ %d ][ %d ] =\n",
					    DTRAN_NAME, nstates, MAX_CHARS);

	    print_array( Ofile, (int *)dtran, nstates, MAX_CHARS );
	    defnext    ( Ofile, DTRAN_NAME );
	}
	else if( Column_compress )		/* column-compressed tables. */
	{
	    i = squash ( Ofile, dtran, nstates, MAX_CHARS, DTRAN_NAME	);
	    cnext      ( Ofile, DTRAN_NAME				);

	    if( Verbose )
		printf("%d bytes required for column-compressed tables\n\n",
		      i 				     /* dtran      */
		      + (nstates * sizeof(int)) ); 	     /* Yy_accept  */
	}
	else				     /* pair-compressed tables */
	{
	    i = pairs( Ofile, (int *)dtran, nstates,
				MAX_CHARS, DTRAN_NAME, Threshold, 0);
	    if( Verbose )
	    {
		/* Figure the space occupied for the various tables. The
		 * Microsoft compiler uses roughly 100 bytes for the yy_next()
		 * subroutine. Column compression does yy_next in line with a
		 * macro so the overhead is negligible.
		 */

		i =   (i        * sizeof(TTYPE ))   /* YysDD arrays	*/
		    + (nstates  * sizeof(TTYPE*))   /* Dtran[]		*/
		    + (nstates  * sizeof(TTYPE ))   /* Yy_accept[]	*/
		    + 100		        ;   /* yy_next()	*/

		printf("%d bytes required for pair-compressed tables\n", i );
	    }

	    pnext( Ofile, DTRAN_NAME );
	}

	pdriver( Ofile, nstates, accept );	/* print rest of driver    */
	tail();					/* and everything following */
						/* the second %%           */
    }
}
/*----------------------------------------------------------------------
 * Head processes everything up to the first %%. Any lines that begin
 * with white space or are surrounded by %{ and %} are passed to the
 * output. All other lines are assumed to be macro definitions.
 * A %% can not be concealed in a %{ %} but it must be anchored at start
 * of line so a %% in a printf statement (for example) is passed to the
 * output correctly. Similarly, a %{ and %} must be the first two characters
 * on the line.
 */

PRIVATE	void head( suppress_output )
int suppress_output;
{
    int  transparent = 0;	/* True if in a %{ %} block */

    if( !suppress_output  && Public )
	fputs( "#define YYPRIVATE\n\n", Ofile );

    if( !No_lines )
	fprintf( Ofile, "#line 1 \"%s\"\n", Input_file_name);

    while( fgets( Input_buf, MAXINP, Ifile) )
    {
	++ Actual_lineno;

	if( !transparent )			/* Don't strip comments */
	    strip_comments( Input_buf );	/* from code blocks.	*/

	if( Verbose > 1 )
	    printf( "h%d: %s", Actual_lineno, Input_buf );

	if( Input_buf[0] == '%' )
	{
	    if( Input_buf[1] == '%' )
	    {
		if( !suppress_output )
		    fputs( "\n", Ofile );
		break;
	    }
	    else
	    {
		if( Input_buf[1] == '{' )			/*}{*/
		    transparent = 1;

		else if( Input_buf[1] == '}' )
		    transparent = 0;

		else
		    lerror(0, "Ignoring illegal %%%c directive\n",
							Input_buf[1] );
	    }
	}
	else if( transparent || isspace(Input_buf[0]) )
	{
	    if( !suppress_output )
		fputs( Input_buf, Ofile );
	}
	else
	{
	    new_macro( Input_buf );
	    if( !suppress_output )
	        fputs( "\n", Ofile );	/* Replace macro def with a blank */
					/* line so that the line numbers  */
					/* won't get messed up.		  */
	}
    }
    if( Verbose > 1 )
	printmacs();
}
/*----------------------------------------------------------------------*/
PRIVATE void strip_comments( string )
char	*string;
{
    /* Scan through the string, replacing C-like comments with space
     * characters. Multiple-line comments are supported.
     */

    static int incomment = 0;

    for(; *string ; ++string )
    {
	if( incomment )
	{
	    if( string[0]=='*' && string[1]=='/' )
	    {
		incomment =  0 ;
		*string++ = ' ';
	    }

	    if( !isspace( *string ) )
		*string = ' ';
	}
	else
	{
	    if( string[0]=='/' && string[1]=='*' )
	    {
		incomment =  1 ;
		*string++ = ' ';
		*string   = ' ';
	    }
	}
    }
}
/*----------------------------------------------------------------------*/
PRIVATE void	tail()
{
    fgets(Input_buf, MAXINP, Ifile);   /* Throw away the line that	*/
				       /* had the %% on it.		*/

    if( !No_lines )
	fprintf( Ofile, "#line %d \"%s\"\n",
			    Actual_lineno + 1, Input_file_name);

    while( fgets(Input_buf, MAXINP, Ifile) )
    {
	if( Verbose > 1 )
	    printf( "t%d: %s", Actual_lineno++, Input_buf );

	fputs(Input_buf, Ofile);
    }
}
