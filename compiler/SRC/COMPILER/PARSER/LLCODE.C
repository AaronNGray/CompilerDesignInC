/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "parser.h"

/*	LLCODE.C	Print the various tables needed for a llama-generated
 *			LL(1) parser. The generated tables are:
 *
 *	Yyd[][]		The parser state machine's DFA transition table. The
 *			horizontal axis is input symbol and the vertical axis is
 *			top-of-stack symbol. Only nonterminal TOS symbols are in
 *			the table. The table contains the production number of a
 *			production to apply or -1 if this is an error
 *			transition.
 *
 *	YydN[]		(N is 1-3 decimal digits). Used for compressed tables
 *			only. Holds the compressed rows.
 *
 *	Yy_pushtab[]	Indexed by production number, evaluates to a list of
 *	YypDD[]		objects to push on the stack when that production is
 *			replaced. The YypDD arrays are the lists of objects
 *			and Yy_pushtab is an array of pointers to those lists.
 *
 *	Yy_snonterm[]	For debugging, indexed by nonterminal, evaluates to the
 *			name of the nonterminal.
 *	Yy_sact[]	Same but for the acts.
 *	Yy_synch[]	Array of synchronization tokens for error recovery.
 *	yy_act()	Subroutine containing the actions.
 *
 * Yy_stok[] is make in stok.c. For the most part, the numbers in these tables
 * are the same as those in the symbol table. The exceptions are the token
 * values, which are shifted down so that the smallest token has the value 1
 * (0 is used for EOI).
 */

#define DTRAN		"Yyd"		/* Name of DFA transition table   */
					/* array in the PARSE_FILE.	  */

/*----------------------------------------------------------------------*/

extern  void tables		P(( void ));			/* public */

static  void fill_row		P(( SYMBOL *lhs ));	/* local */
static  void make_pushtab	P(( SYMBOL *lhs ));
static  void make_yy_pushtab	P(( void ));
static  void make_yy_dtran	P(( void ));
static  void make_yy_synch	P(( void ));
static  void make_yy_snonterm	P(( void ));
static  void make_yy_sact	P(( void ));
static  void make_acts		P(( SYMBOL *lhs ));
static  void make_yy_act	P(( void  ));
/*----------------------------------------------------------------------*/
PRIVATE int	*Dtran;		/* Internal representation of the parse table.
				 * Initialization in make_yy_dtran() assumes
				 * that it is an int (it calls memiset).
				 */
/*----------------------------------------------------------------------*/
PUBLIC void	tables()
{
	/* Print the various tables needed by the parser.  */

	make_yy_pushtab();
	make_yy_dtran();
	make_yy_act();
	make_yy_synch();
	make_yy_stok();
	make_token_file();

        output( "\n#ifdef YYDEBUG\n");

	make_yy_snonterm();
	make_yy_sact();

        output( "\n#endif\n" );
}
/*----------------------------------------------------------------------
 *  fill_row()
 *
 *  Make one row of the parser's DFA transition table.
 *  Column 0 is used for the EOI condition; other columns
 *  are indexed by nonterminal (with the number normalized
 *  for the smallest nonterminal). That is, the terminal
 *  values in the symbol table are shifted downwards so that
 *  the smallest terminal value is 1 rather than MINTERM.
 *  The row indexes are adjusted in the same way (so that
 *  row 0 is used for MINNONTERM).
 *
 *  Note that the code assumes that Dtran consists of  byte-size
 *  cells.
 */

PRIVATE	void	fill_row( lhs )
SYMBOL	*lhs; 			/* Current left-hand side  */
{
    PRODUCTION  *prod;
    int 	*row;
    int   	i;
    int		rowsize;

    if( !ISNONTERM(lhs) )
	return;

    rowsize = USED_TERMS + 1;
    row     = Dtran + ((i = ADJ_VAL(lhs->val)) * rowsize );

    for( prod = lhs->productions; prod ; prod = prod->next )
    {
	next_member( NULL );
	while( (i = next_member(prod->select)) >= 0 )
	{
	    if( row[i] == -1 )
		row[i] = prod->num;
	    else
	    {
		error(NONFATAL, "Grammar not LL(1), select-set conflict in " );
		error(NOHDR,    "<%s>, line %d\n", lhs->name, lhs->lineno    );
	    }
	}
    }
}
/*----------------------------------------------------------------------*/
PRIVATE	void	make_pushtab( lhs )
SYMBOL	*lhs;
{
     /* Make the pushtab. The right-hand sides are output in reverse order
      * (to make the pushing easier) by stacking them and then printing
      * items off the stack.
      */

    register int  i ;
    PRODUCTION    *prod  ;
    SYMBOL	      **sym  ;
    SYMBOL	      *stack[ MAXRHS ], **sp;

    sp = &stack[-1] ;
    for( prod = lhs->productions ; prod ; prod = prod->next )
    {
	output( "YYPRIVATE int Yyp%02d[]={ ", prod->num );

	for( sym = prod->rhs, i = prod->rhs_len ; --i >= 0  ;)
	    *++sp = *sym++ ;

	for(; INBOUNDS(stack,sp) ; output("%d, ", (*sp--)->val) )
	    ;

	output(  "0 };\n", prod->rhs[0] );
    }
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PRIVATE	void	make_yy_pushtab()
{
    /* Print out yy_pushtab. */

    register int	i;
    register int	maxprod = Num_productions - 1;
    static	 char	*text[] =
    {
	"The YypNN arrays hold the right-hand sides of the productions, listed",
	"back to front (so that they are pushed in reverse order), NN is the",
	"production number (to be found in the symbol-table listing output",
	"with a -s command-line switch).",
	"",
	"Yy_pushtab[] is indexed by production number and points to the",
	"appropriate right-hand side (YypNN) array.",
	NULL
    };

    comment( Output, text );
    ptab   ( Symtab, (ptab_t) make_pushtab, NULL, 0 );

    output(  "\nYYPRIVATE int  *Yy_pushtab[] =\n{\n");

    for( i = 0; i < maxprod; i++ )
	    output(  "\tYyp%02d,\n", i );

    output( "\tYyp%02d\n};\n", maxprod );
}
/*----------------------------------------------------------------------*/
PRIVATE	void	make_yy_dtran( )
{
    /* Print the DFA transition table.  */

    int		i;
    int		nterms, nnonterms;  /* number of terminals and nonterminals */
    static char	*text[] =
    {
	"Yyd[][] is the DFA transition table for the parser. It is indexed",
	"as follows:",
	"",
	"                Input symbol",
	"          +----------------------+",
	"       L  |  Production number   |",
	"       H  |       or YYF         |",
	"       S  |                      |",
	"          +----------------------+",
	"",
	"The production number is used as an index into Yy_pushtab, which",
	"looks like this:",
	"",
	"        Yy_pushtab      YypDD:",
	"        +-------+       +----------------+",
	"        |   *---|------>|                |",
	"        +-------+       +----------------+",
	"        |   *---|---->",
	"        +-------+",
	"        |   *---|---->",
	"        +-------+",
	"",
	"YypDD is the tokenized right-hand side of the production.",
	"Generate a symbol table listing with llama's -l command-line",
	"switch to get both production numbers and the meanings of the",
	"YypDD string contents.",
	NULL
    };

    nterms    = USED_TERMS + 1;			/* +1 for EOI */
    nnonterms = USED_NONTERMS;

    i = nterms * nnonterms;			/* Number of cells in array */

    if( !(Dtran = (int *) malloc(i * sizeof(*Dtran)) ))  /* number of bytes */
	ferr("Not enough memory to print DFA transition table.\n");

    memiset( Dtran, -1, i );	        /* Initialize Dtran to all failures */
    ptab( Symtab, (ptab_t) fill_row, NULL, 0 );	/* & fill nonfailure transitions    */

    comment( Output , text );	   	/* Print header comment */

    if( Uncompressed )
    {
	fprintf( Output,"YYPRIVATE YY_TTYPE  %s[ %d ][ %d ] =\n",
						DTRAN, nnonterms, nterms );

	print_array( Output, Dtran, nnonterms, nterms );
	defnext    ( Output, DTRAN );

	if( Verbose )
	    printf("%d bytes required for tables\n", i * sizeof(YY_TTYPE) );
    }
    else
    {
	i = pairs( Output, Dtran, nnonterms, nterms, DTRAN, Threshold, 1);
	    pnext( Output, DTRAN );

	if( Verbose )
	    printf("%d bytes required for compressed tables\n",
		  (i * sizeof(YY_TTYPE)) + (nnonterms * sizeof(YY_TTYPE*)));
    }
    output("\n\n");
}
/*----------------------------------------------------------------------*/
PRIVATE	void	make_yy_synch()
{
    int	 mem ;	/* current member of synch set  */
    int	 i;	/* number of members in set	*/

    static char *text[] =
    {
	"Yy_synch[] is an array of synchronization tokens. When an error is",
	"detected, stack items are popped until one of the tokens in this",
	"array is encountered. The input is then read until the same item is",
	"found. Then parsing continues.",
	NULL
    };

    comment( Output, text );
    output( "YYPRIVATE int  Yy_synch[] =\n{\n" );

    i = 0;
    for( next_member(NULL); (mem = next_member(Synch)) >= 0 ;)
    {
	output( "\t%s,\n", Terms[mem]->name );
	++i;
    }

    if( i == 0 )		/* No members in synch set */
	output( "\t_EOI_\n" );

    output( "\t-1\n};\n" );
    next_member( NULL );
}
/*----------------------------------------------------------------------*/
PRIVATE	void	make_yy_snonterm()
{
    register int  i;

    static char	*the_comment[] =
    {
	"Yy_snonterm[] is used only for debugging. It is indexed by the",
	"tokenized left-hand side (as used for a row index in Yyd[]) and",
	"evaluates to a string naming that left-hand side.",
	NULL
    };

    comment( Output, the_comment );

    output(  "char *Yy_snonterm[] =\n{\n");

    for( i = MINNONTERM; i <= Cur_nonterm; i++ )
    {
	if( Terms[i] )
	    output(  "\t/* %3d */   \"%s\"", i, Terms[i]->name );

	if( i != Cur_nonterm )
		outc( ',' )

	outc( '\n' );
    }

    output(  "};\n\n" );
}
/*----------------------------------------------------------------------*/
PRIVATE	void	make_yy_sact()
{
    /* This subroutine generates the subroutine that implements the actions.  */

    register int  i;
    static char	*the_comment[] =
    {
	"Yy_sact[] is also used only for debugging. It is indexed by the",
	"internal value used for an action symbol and evaluates to a string",
	"naming that token symbol.",
	NULL
    };

    comment( Output, the_comment );
    output("char *Yy_sact[] =\n{\n\t" );

    if( Cur_act < MINACT )
	output("NULL    /* There are no actions */");
    else
	for( i = MINACT; i <= Cur_act; i++ )
	{
	    output( "\"{%d}\"%c", i - MINACT, i < Cur_act ? ',' : ' ' );
	    if( i % 10 == 9 )
		    output("\n\t");
	}

    output("\n};\n");
}
/*----------------------------------------------------------------------*/
PRIVATE	void make_acts( lhs )
SYMBOL	*lhs;			/* Current left-hand side  */
{
    /* This subroutine is called indirectly from yy_act, through the subroutine
     * ptab(). It prints the text associated with one of the acts.
     */

    char *p;
    int  num;		/* The N in $N */
    char *do_dollar();
    char fname[80], *fp;
    int  i;

    if( !lhs->string )
	    return;

    output(  "        case %d:\n", lhs->val );

    if ( No_lines )
	output(  "\t\t" );
    else
	output(  "#line %d \"%s\"\n\t\t", lhs->lineno, Input_file_name );

    for( p = lhs->string ; *p ; )
    {
	if( *p == '\r' )
	    continue;

	if( *p != '$' )
	    output( "%c", *p++ );
	else
	{
	    /* Skip the attribute reference. The if statement handles $$ the
	     * else clause handles the two forms: $N and $-N, where N is a
	     * decimal number. When you hit the do_dollar call (in the output()
	     * call), "num" holds the number associated with N, or DOLLAR_DOLLAR
	     * in the case of $$.
	     */

	    if( *++p != '<' )
		*fname = '\0';
	    else
	    {
		++p;		/* skip the < */
		fp = fname;

		for(i=sizeof(fname); --i>0  && *p && *p != '>'; *fp++ = *p++ )
		    ;
		*fp = '\0';

		if( *p == '>' )
		    ++p;
	    }

	    if( *p == '$' )
	    {
		num = DOLLAR_DOLLAR;
		++p;
	    }
	    else
	    {
		num = atoi( p );
		if( *p == '-' )
		    ++p ;
		while( isdigit(*p) )
		    ++p ;
	    }

	    output( "%s", do_dollar(num, 0, 0, NULL, fname) );
	}
    }

    output(  "\n                break;\n" );
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PRIVATE	void	make_yy_act()
{
    /* Print all the acts inside a subroutine called yy_act() */

    static char	*comment_text[] =
    {
	"Yy_act() is the action subroutine. It is passed the tokenized value",
	"of an action and executes the corresponding code.",
	NULL
    };

    static char	*top[] =
    {
	"YYPRIVATE int yy_act( actnum )",
	"int actnum;",
	"{",
	"    /* The actions. Returns 0 normally but a nonzero error code can",
	"     * be returned if one of the acts causes the parser to terminate",
	"     * abnormally.",
	"     */",
	"",
	"     switch( actnum )",
	"     {",
	NULL
    };
    static char	*bottom[] =
    {
	"     default:  printf(\"INTERNAL ERROR: Illegal act number (%s)\\n\",",
	"                                                             actnum);",
	"               break;",
	"     }",
	"     return 0;",
	"}",
	NULL
    };

    comment( Output, comment_text		 );
    printv ( Output, top 			 );
    ptab   ( Symtab, (ptab_t) make_acts, NULL, 0 );
    printv ( Output, bottom 			 );
}
