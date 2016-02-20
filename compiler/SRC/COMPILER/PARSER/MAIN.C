/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>
#include <malloc.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <process.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>

#ifdef LLAMA
#	define  ALLOCATE
#	include "parser.h"
#	undef   ALLOCATE
#else
#	define  ALLOCATE
#	include "parser.h"
#	undef   ALLOCATE
#endif

#if (0 MSC(+1) && !defined(_WIN32))
    #pragma comment(exestr, "(C)" __DATE__ "Allen Holub. All rights reserved.")
#endif

PRIVATE   int   Warn_exit     = 0;	/* Set to 1 if -W on command line  */
PRIVATE   int	Num_warnings  = 0;	/* Total warnings printed	   */
PRIVATE	  char  *Output_fname = "????"; /* Name of the output file	   */
PRIVATE   FILE *Doc_file      = NULL;   /* Error log & machine description */

#ifdef MAP_MALLOC
  PRIVATE int	Malloc_chk     = 1;	/* Run-time malloc() checking      */
  PRIVATE int	Malloc_verbose = 0;	/*     " use verbose diagnostics   */
#endif

#define VERBOSE(str)  if(Verbose){  printf( "%s:\n", (str));  }else
/*----------------------------------------------------------------------*/
void onintr	  P(( void 			));		/* local */
void parse_args	  P(( int argc,  char **argv	));
int  do_file	  P(( void			));
void symbols	  P(( void			));
void statistics	  P(( FILE *fp			));
void tail	  P(( void			));

void output	  P(( char *fmt, ...		));		/* public */
void lerror	  P(( int fatal, char *fmt, ...	));
void error	  P(( int fatal, char *fmt, ...	));
char *open_errmsg P(( void 			));
char *do_dollar	  P(( int num, int rhs_size, int lineno, PRODUCTION *prod, \
								char *fname));
/*----------------------------------------------------------------------
 * There are two versions of the following subroutines--used in do_file(),
 * depending on whether this is llama or occs.

 *
 * subroutine:		llama version in:	occs version in:
 *
 * file_header()	lldriver.c		yydriver.c
 * code_header()	lldriver.c		yydriver.c
 * driver()		lldriver.c		yydriver.c
 * tables()		llcode.c		yycode.c
 * patch()		-----			yypatch.c
 * select()		llselect.c		------
 * do_dollar()		lldollar.c		yydollar.c
 *
 * Also, several part of this file are compiled only for llama, others only for
 * occs. The llama-specific parts are arguments to LL() macros, the occs-
 * specific parts are in OX() macros. We'll look at what the occs-specific
 * parts actually do in the next Chapter.
 *----------------------------------------------------------------------
 */
PUBLIC	void main( argc, argv )
char	**argv;
{
    //MSC( _amblksiz = 2048; )	/* Declared in Microsoft C's malloc.h.        */
    				/* Controls size of malloc() allocation unit. */
    Output                      = stdout; /* Output stream.		      */

    signon();			/* Print sign on message		   */
    signal( SIGINT, 		/* Close output files on Ctrl-Break.	   */
	MSC((void(*)(int))) onintr );
    parse_args( argc, argv );

# ifdef MAP_MALLOC
       malloc_checking( Malloc_chk, Malloc_verbose );
# endif

    if( Debug && !Symbols )
	Symbols = 1;

    OX( if( Make_parser )						)
    OX( {								)
    OX(     if( Verbose == 1 )						)
    OX(     {								)
    OX(	        if( !(Doc_file = fopen( DOC_FILE, "w") ) )		)
    OX(	            ferr( "Can't open log file %s\n", DOC_FILE );	)
    OX(     }								)
    OX(     else if( Verbose > 1 )					)
    OX(		Doc_file = stderr;					)
    OX( }								)

    if( Use_stdout )
    {
	Output_fname = "/dev/tty" ;
	Output       = stdout;
    }
    else
    {
	OX( Output_fname = !Make_parser ? ACT_FILE : PARSE_FILE ; )
	LL( Output_fname = PARSE_FILE; 				  )

	if( (Output = fopen( Output_fname, "w")) == NULL )
	    error( FATAL, "Can't open output file %s: %s\n",
						Output_fname, open_errmsg() );
    }

    if( (yynerrs = do_file()) == 0)	/* Do all the work  */
    {
	if( Symbols )
	    symbols();			/* Print the symbol table	  */

	statistics( stdout );		/* And any closing-up statistics. */

	if( Verbose && Doc_file )
	{
	    OX(  statistics( Doc_file );   )

#ifdef MAP_MALLOC
  	    printf("\n%lu bytes (%luK) memory used by malloc\n",
  				    memory_used(), memory_used()/1024L );
#endif
	}
    }
    else
    {
	if( Output != stdout )
	{
	    fclose( Output );
	    if( unlink( Output_fname ) == -1 )
		perror( Output_fname );
	}
    }

    /* Exit with  the number of hard errors (or, if -W was specified, the sum
     * of the hard errors and warnings) as an exit status. Doc_file and Output
     * are closed implicitly by exit().
     */

    exit( yynerrs + (Warn_exit ? Num_warnings : 0) );
}
/*----------------------------------------------------------------------*/
PRIVATE void onintr()			/* SIGABRT (Ctrl-Break, ^C) Handler */
{
    if( Output != stdout )		/* Destroy parse file so that a */
    {					/* subsequent compile will fail */
	fclose( Output );
	unlink( Output_fname );
    }

    exit( EXIT_USR_ABRT );
}
/*----------------------------------------------------------------------*/
PRIVATE	void parse_args( argc, argv )
int	argc;
char	**argv;
{
    /* Parse the command line, setting global variables as appropriate */

    char	*p;
    static char name_buf[80];	       /* Use to assemble default file names */

    static char	*usage_msg[] =
    {
#ifdef LLAMA
        "Usage is: llama [-switch] file",
        "Create an LL(1) parser from the specification in the",
	"input file. Legal command-line switches are:",
	"",
        "-cN      use N as the pairs threshold when (C)ompressing",
	"-D       enable (D)ebug mode in yyparse.c (implies -s)",
        "-f       (F)ast, uncompressed, tables",
#else
        "Usage is:  occs [-switch] file",
	"",
        "\tCreate an LALR(1) parser from the specification in the",
	"\tinput file. Legal command-line switches are:",
	"",
        "-a       Output actions only (see -p)",
	"-D       enable (D)ebug mode in yyparse.c (implies -s)",
#endif
	"-g       make static symbols (G)lobal in yyparse.c",
	"-l       suppress #(L)ine directives",
	"-m<file> use <file> for parser te(M)plate",
	"-p       output parser only (can be used with -T also)",
	"-s       make (s)ymbol table",
	"-S       make more-complete (S)ymbol table",
	"-t       print all (T)ables (and the parser) to standard output",
	"-T	  move large tables from yyout.c to yyoutab.c",
	"-v       print (V)erbose diagnostics (including symbol table)",
	"-V       more verbose than -v. Implies -t, & yyout.doc goes to stderr",
	"-w       suppress all warning messages",
	"-W       warnings (as well as errors) generate nonzero exit status",
#ifdef MAP_MALLOC
  	"-M       Disable malloc checking at run time\n",
#endif
	NULL
    };

    /* Note that all global variables set by command-line switches are declared
     * in parser.h. Space is allocated because a #define ALLOC is present at
     * the top of the current file.
     */

    for( ++argv,--argc; argc && *(p = *argv) == '-'; ++argv, --argc )
    {
	while( *++p )
	{
	    switch( *p )
	    {
	OX(  case 'a':  Make_parser  = 0;		)
	OX(	        Template     = ACT_TEMPL;	)
	OX(	        break;				)

	    case 'D':  Debug 	    = 1;	break;
	    case 'g':  Public	    = 1;	break;
	LL( case 'f':  Uncompressed = 1; 	break;		)
	    case 'l':  No_lines	    = 1;	break;
	    case 'm':  Template     = p + 1;	goto out;

#ifdef MAP_MALLOC
  	    case 'M':  Malloc_chk   = 0;	break;
#endif

	OX( case 'p':  Make_actions = 0;	break;		)
	    case 's':  Symbols      = 1;	break;
	    case 'S':  Symbols      = 2;	break;
	    case 't':  Use_stdout   = 1; 	break;
	    case 'T':  Make_yyoutab = 1; 	break;
	    case 'v':  Verbose	    = 1;	break;
	    case 'V':  Verbose	    = 2;	break;
	    case 'w':  No_warnings  = 1;	break;
	    case 'W':  Warn_exit    = 1;	break;
	LL( case 'c':  Threshold = atoi( ++p );			)
	LL(	       while( *p && isdigit( p[1] ) )		)
	LL(		    ++p;				)
	LL(	       break;					)
	    default :
		       fprintf(stderr, "<-%c>: illegal argument\n", *p);
		       printv (stderr, usage_msg );
		       exit( EXIT_ILLEGAL_ARG );
	    }
	}
out: ;

    }

    if( Verbose > 1 )
	Use_stdout = 1;

    if( argc <= 0 )			/* Input from standard input	*/
	No_lines = 1;

    else if( argc > 1 )
    {
	fprintf( stderr, "Too many arguments.\n" );
	printv ( stderr, usage_msg );
	exit   ( EXIT_TOO_MANY );
    }
    else 				/* argc == 1, input from file   */
    {
	if( ii_newfile( Input_file_name = *argv ) < 0 )
	{
	    sprintf( name_buf, "%0.70s.%s", *argv, DEF_EXT );

	    if( ii_newfile( Input_file_name = name_buf ) < 0 )
		error( FATAL, "Can't open input file %s or %s: %s\n",
						*argv, name_buf, open_errmsg());
	}
    }
}
/*----------------------------------------------------------------------*/
PRIVATE	int	do_file()
{
    /* Process the input file. Return the number of errors.  */

    struct timeb start_time, end_time ;
    long	 time;
    void 	 nows P((void));	/* declared in parser.lex */

    llinit();
    OX(yyinit());
    ftime( &start_time );    /* Initialize times now so that the difference   */
    end_time = start_time;   /* between times will be 0 if we don't build the */
			     /* tables. Note that I'm using structure assign- */
			     /* ment here.			              */
    init_acts  ();	     /* Initialize the action code.		      */
    file_header();  	     /* Output #defines that you might want to change */

    VERBOSE( "parsing" );

    nows      ();         /* Make lex ignore white space until ws() is called */
    yyparse   ();         /* Parse the entire input file		      */

    if( !( yynerrs || problems()) )	  /* If no problems in the input file */
    {
	VERBOSE( "analyzing grammar" );

	    first ();		    /* Find FIRST sets,                       */
	LL( follow(); )		    /* FOLLOW sets,                           */
	LL( select(); ) 	    /* and ll(1) select sets if this is llama */

	code_header();		    /* Print various #defines to output file  */
	OX( patch(); )		    /* Patch up the grammar (if this is occs) */
				    /* and output the actions.                */

	ftime( &start_time );

	if( Make_parser )
	{
	    VERBOSE( "making tables" );
	    tables (); 		                      /* generate the tables, */
	}

	ftime  ( &end_time        );
	VERBOSE( "copying driver" );

	driver(); 					       /* the parser, */

	if( Make_actions )
	    tail();  		      /* and the tail end of the source file. */
    }

    if( Verbose )
    {
	time  = (  end_time.time * 1000) +   end_time.millitm ;
	time -= (start_time.time * 1000) + start_time.millitm ;
	printf( "time required to make tables: %ld.%-03ld seconds\n",
						(time/1000), (time%1000));
    }

    return yynerrs;
}
PRIVATE void	symbols( )			    /* Print the symbol table */
{
    FILE  *fd;

    if( !(fd = fopen( SYM_FILE, "w")) )
	perror( SYM_FILE );
    else
    {
	print_symbols ( fd );
	fclose        ( fd );
    }
}
/*----------------------------------------------------------------------*/
PRIVATE void	statistics( fp )
FILE	*fp;
{
    /* Print various statistics
    */

    int	conflicts;		/* Number of parse-table conflicts */

    if( Verbose )
    {
	 fprintf (fp, "\n");
	 fprintf (fp, "%4d/%-4d terminals\n",     USED_TERMS,    NUMTERMS   );
	 fprintf (fp, "%4d/%-4d nonterminals\n",  USED_NONTERMS, NUMNONTERMS);
	 fprintf (fp, "%4d/%-4d productions\n",   Num_productions, MAXPROD  );
LL(	 fprintf (fp, "%4d      actions\n",       (Cur_act - MINACT) +1     );)
OX(	 lr_stats(fp );				      	      		      )
    }

    LL( conflicts = 0;			)
    OX( conflicts = lr_conflicts(fp);   )

    if( fp == stdout )
	fp = stderr;

    if( Num_warnings - conflicts > 0 )
	fprintf(fp, "%4d      warnings\n", Num_warnings - conflicts);

    if(yynerrs)
	fprintf(fp, "%4d      hard errors\n", yynerrs   );
}
/*----------------------------------------------------------------------*/
ANSI( PUBLIC	void output( char *fmt, ... )	)
KnR ( PUBLIC	void output(       fmt      )	)
KnR ( char	*fmt;				)
{
    /* Works like printf(), but writes to the output file. See also: the
     * outc() macro in parser.h
     */

    va_list   args;
    va_start( args,   fmt );
    vfprintf( Output, fmt, args );
}
/*----------------------------------------------------------------------*/
ANSI( PUBLIC	void document( char *fmt, ... )	)
KnR ( PUBLIC	void document( fmt )		)
KnR ( char	*fmt;				)
{
    /* Works like printf() but writes to yyout.doc (provided that the file
     * is being created.
     */

    va_list   args;

    if( Doc_file )
    {
	va_start( args,   fmt );
	vfprintf( Doc_file, fmt, args );
    }
}

ANSI( void document_to( FILE *fp )	)
KnR ( void document_to(       fp )	)
KnR ( FILE *fp;				)
{
    /* Change document-file output to the indicated stream, or to previous
     * stream if fp=NULL.
     */

     static FILE *oldfp;

     if( fp )
     {
	 oldfp    = Doc_file;
	 Doc_file = fp;
     }
     else
	 Doc_file = oldfp;
}
/*----------------------------------------------------------------------*/
ANSI( PUBLIC	void lerror(int fatal, char *fmt, ... )	)
KnR ( PUBLIC	void lerror(    fatal,       fmt      )	)
KnR ( int	fatal;					)
KnR ( char	*fmt;					)
{
    /* This error-processing routine automatically generates a line number for
     * the error. If "fatal" is true, exit() is called.
     */

    va_list	args;
    extern int	yylineno;

    if( fatal == WARNING )
    {
	++Num_warnings;
	if( No_warnings )
	    return;
	fprintf( stdout, "%s WARNING (%s, line %d): ", PROG_NAME,
					      Input_file_name, yylineno );
    }
    else if( fatal != NOHDR )
    {
	++yynerrs;
	fprintf( stdout, "%s ERROR   (%s, line %d): ", PROG_NAME,
					      Input_file_name, yylineno );
    }

    va_start( args,   fmt);
    vfprintf( stdout, fmt, args );
    fflush  ( stdout );

#   ifndef LLAMA
	if( Verbose && Doc_file )
	{
	    if( fatal != NOHDR )
	         fprintf ( Doc_file, "%s (line %d) ",
			fatal==WARNING ? "WARNING" : "ERROR", yylineno);
	    vfprintf( Doc_file, fmt, args );
	}
#   endif

    if( fatal == FATAL )
	exit( EXIT_OTHER );
}

ANSI( PUBLIC	void error(int fatal, char *fmt, ... )	)
KnR ( PUBLIC	void error(    fatal,       fmt      )	)
KnR ( int	fatal;					)
KnR ( char	*fmt;					)
{
    /* This error routine works like lerror() except that no line number is
     * generated. The global error count is still modified, however.
     */

    va_list  args;
    char     *type;

    if( fatal == WARNING )
    {
	++Num_warnings;
	if( No_warnings )
	    return;
	type = "WARNING: ";
    }
    else if( fatal != NOHDR )		/* if NOHDER is true, just act  */
    {					/* like printf().		*/
	++yynerrs;
	type = "ERROR: ";
	fprintf( stdout, type );
    }

    va_start ( args,   fmt  	 );
    vfprintf ( stdout, fmt, args );
    fflush   ( stdout 		 );

    OX( if( Verbose && Doc_file )			)
    OX( {						)
    OX(	    fprintf  ( Doc_file, type );		)
    OX(	    vfprintf ( Doc_file, fmt, args );		)
    OX( }						)

    if( fatal == FATAL )
	exit( EXIT_OTHER );
}

PUBLIC char	*open_errmsg()
{
    /* Return an error message that makes sense for a bad open */

    switch( errno )	/* defined in stdlib.h */
    {
    case EACCES:	return "File is read only or a directory";
    case EEXIST:	return "File already exists";
    case EMFILE:	return "Too many open files";
    case ENOENT:	return "File not found";
    default:		return "Reason unknown";
    }
}
/*----------------------------------------------------------------------*/
PRIVATE	void tail()
{
    /* Copy the remainder of input file to standard output. Yyparse will have
     * terminated with the input pointer just past the %%. Attribute mapping
     * ($$ to Yyval, $N to a stack reference, etc.) is done by the do_dollar()
     * call.
     *
     * On entry, the parser will have read one token too far, so the first
     * thing to do is print the current line number and lexeme.
     */

    extern int	yylineno ;	/* LeX generated */
    extern char *yytext  ;	/* LeX generated */
    int		c, i, sign;
    char	fname[80], *p;	/* field name in $<...>n */

    output( "%s", yytext);		/* Output newline following %% */

    if( !No_lines )
	output( "\n#line %d \"%s\"\n", yylineno, Input_file_name );

    ii_unterm();			/* Lex will have terminated yytext */

    while( (c = ii_advance()) != 0 )
    {
	if( c == -1 )
	{
	    ii_flush(1);
	    continue;
	}
	else if( c == '$' )
	{
	    ii_mark_start();

	    if( (c = ii_advance()) != '<' )
		*fname = '\0';
	    else				/* extract name in $<foo>1 */
	    {
		p = fname;
		for(i=sizeof(fname); --i > 0  &&  (c=ii_advance()) != '>';)
		    *p++ = c;
		*p++ = '\0';

		if( c == '>' )		/* truncate name if necessary */
		    c = ii_advance();
	    }

	    if( c == '$' )
		output( do_dollar( DOLLAR_DOLLAR, -1, 0, NULL, fname ) );
	    else
	    {

		if( c != '-' )
		    sign = 1 ;
		else
		{
		    sign = -1;
		    c = ii_advance();
		}

		for( i = 0; isdigit(c); c = ii_advance() )
		    i = (i * 10) + (c - '0');

		ii_pushback(1);
		output( do_dollar(i * sign, -1, ii_lineno(), NULL, fname ) );
	    }
	}
	else if( c != '\r' )
	    outc( c );
    }
}
