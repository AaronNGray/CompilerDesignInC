/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "parser.h"

/*----------------------------------------------------------------------*/

void patch    	    P(( void		));			/* public */

void dopatch	    P(( SYMBOL *sym	));			/* local  */
void print_one_case P(( int case_val, unsigned char *action, \
		     int rhs_size, int lineno, struct _prod_ *prod ));

/*----------------------------------------------------------------------*/

PRIVATE int	Last_real_nonterm ;	/* This is the number of the last     */
					/* nonterminal to appear in the input */
					/* grammar [as compared to the ones   */
					/* that patch() creates].	      */

/*-------------------------------------------------------------------*/

#ifdef NEVER /*-------------------------------------------------------*/
  | The symbol table, though handy, needs some shuffling around to make
  | it useful in an LALR application. (It was designed to make it easy
  | for the parser). First of all, all actions must be applied on a
  | reduction rather than a shift. This means that imbedded actions
  | (like this):
  |
  |	foo : bar { act(1); } cow { act(2); };
  |
  | have to be put in their own productions (like this):
  |
  |	foo : bar {0} cow   { act(2); };
  |	{0} : /* epsilon */ { act(1); };
  |
  | Where {0} is treated as a nonterminal symbol; Once this is done, you
  | can print out the actions and get rid of the strings. Note that,
  | since the new productions go to epsilon, this transformation does
  | not affect either the first or follow sets.
  |
  | Note that you don't have to actually add any symbols to the table;
  | you can just modify the values of the action symbols to turn them
  | into nonterminals.
  |
#endif /*------------------------------------------------------------*/

PUBLIC void patch()
{
    /*  This subroutine does several things:
     *
     *	    * It modifies the symbol table as described in the text.
     *	    * It prints the action subroutine and deletes the memory associated
     *	      with the actions.
     *
     * This is not a particularly good subroutine from a structured programming
     * perspective because it does two very different things at the same time.
     * You save a lot of code by combining operations, however.
     */

    void dopatch();

    static char *top[] =
    {
	"",
	"yy_act( yypnum, yyvsp )",
	"int		yypnum;			 /* production number   */",
	"YYSTYPE	*yyvsp;                  /* value-stack pointer */",
	"{",
	"    /* This subroutine holds all the actions in the original input",
	"     * specification. It normally returns 0, but if any of your",
	"     * actions return a non-zero number, then the parser halts",
	"     * immediately, returning that nonzero number to the calling",
	"     * subroutine.",
	"     */",
	"",
	"    switch( yypnum )",
	"    {",
	NULL
    };

    static char *bot[] =
    {
	"",
	"#     ifdef YYDEBUG",
	"        default: yycomment(\"Production %d: no action\\n\", yypnum);",
	"             break;",
	"#     endif",
	"    }",
	"",
	"    return 0;",
	"}",
	NULL
    };

    Last_real_nonterm = Cur_nonterm;

    if( Make_actions )
	printv( Output, top );

    ptab( Symtab, (ptab_t)dopatch, NULL, 0 );

    if( Make_actions )
	printv( Output, bot );
}


/*----------------------------------------------------------------------*/
  /*
   * dopatch() does two things, it modifies the symbol table for use with
   * occs and prints the action symbols. The alternative is to add another
   * field to the PRODUCTION structure and keep the action string there,
   * but this is both a needless waste of memory and an unnecessary
   * complication because the strings will be attached to a production number
   * and once we know that number, there's not point in keeping them around.
   *
   * If the action is the rightmost one on the rhs, it can be discarded
   * after printing it (because the action will be performed on reduction
   * of the indicated production), otherwise the transformation indicated
   * earlier is performed (adding a new nonterminal to the symbol table)
   * and the string is attached to the new nonterminal).
   */

PRIVATE void dopatch(sym)
SYMBOL	*sym;
{
    PRODUCTION  *prod;	   /* Current right-hand side of sym		    */
    SYMBOL	**pp ;	   /* Pointer to one symbol on rhs		    */
    SYMBOL      *cur ;	   /* Current element of right-hand side	    */
    int		i    ;

    if( !ISNONTERM(sym)  ||  sym->val > Last_real_nonterm )
    {
	/* If the current symbol isn't a nonterminal, or if it is a nonterminal
	 * that used to be an action (one that we just transformed), ignore it.
	 */
	return;
    }

    for( prod = sym->productions; prod ; prod = prod->next )
    {
	if( prod->rhs_len == 0 )
	    continue;

	pp  = prod->rhs + (prod->rhs_len - 1);
	cur = *pp;

	if( ISACT(cur) )			/* Check rightmost symbol */
	{
	    print_one_case(  prod->num, cur->string, --(prod->rhs_len),
					cur->lineno, prod );

	    delsym  ( (HASH_TAB*) Symtab, (BUCKET*) cur );
	    free    ( cur->string );
	    freesym ( cur );
	    *pp-- = NULL;
	}
			/* cur is no longer valid because of the --pp above. */
			/* Count the number of nonactions in the right-hand  */
			/* side	and modify imbedded actions		     */

	for(i = (pp - prod->rhs) + 1; --i >= 0;  --pp )
	{
	    cur = *pp;

	    if( !ISACT(cur) )
		continue;

	    if( Cur_nonterm >= MAXNONTERM )
		error(1,"Too many nonterminals & actions (%d max)\n", MAXTERM);
	    else
	    {
		/* Transform the action into a nonterminal. */

		Terms[ cur->val = ++Cur_nonterm ] = cur ;

		cur->productions = (PRODUCTION*) malloc( sizeof(PRODUCTION) );
		if( !cur->productions )
		    error(1, "INTERNAL [dopatch]: Out of memory\n");

		print_one_case( Num_productions,  /* Case value to use.	      */
				cur->string,	  /* Source code.	      */
				pp - prod->rhs,	  /* # symbols to left of act.*/
				cur->lineno,	  /* Input line # of code.    */
				prod
			      );

		/* Once the case is printed, the string argument can be freed.*/

    		free( cur->string );
		cur->string               = NULL;
		cur->productions->num     = Num_productions++ ;
		cur->productions->lhs     = cur ;
		cur->productions->rhs_len = 0;
		cur->productions->rhs[0]  = NULL;
		cur->productions->next    = NULL;
		cur->productions->prec    = 0;

		/* Since the new production goes to epsilon and nothing else,
		 * FIRST(new) == { epsilon }. Don't bother to refigure the
		 * follow sets because they won't be used in the LALR(1) state-
		 * machine routines [If you really want them, call follow()
		 * again.]
		 */

		cur->first = newset();
		ADD( cur->first, EPSILON );
	    }
	}
    }
}

PRIVATE	void print_one_case( case_val, action, rhs_size, lineno, prod )
int		case_val;  	    /* Numeric value attached to case itself.*/
unsigned char	*action;   	    /* Source Code to execute in case.	     */
int		rhs_size;  	    /* Number of symbols on right-hand side. */
int		lineno;	   	    /* input line number (for #lines).	     */
PRODUCTION	*prod;	   	    /* Pointer to right-hand side.    	     */
{
    /* Print out one action as a case statement. All $-specifiers are mapped
     * to references to the value stack: $$ becomes Yy_vsp[0], $1 becomes
     * Yy_vsp[-1], etc. The rhs_size argument is used for this purpose.
     * [see do_dollar() in yydollar.c for details].
     */

    int 	num, i;
    char 	*do_dollar();		/* source found in yydollar.c	*/
    extern char *production_str();	/* source found in acts.c 	*/
    char	fname[40], *fp;		/* place to assemble $<fname>1  */

    if( !Make_actions )
	return;

    output("\n    case %d: /*  %s  */\n\n\t", case_val, production_str(prod) );

    if( !No_lines )
	output("#line %d \"%s\"\n\t", lineno, Input_file_name );

    while( *action )
    {
	if( *action != '$' )
	    output( "%c", *action++ );
	else
	{
	    /* Skip the attribute reference. The if statement handles $$ the
	     * else clause handles the two forms: $N and $-N, where N is a
	     * decimal number. When we hit the do_dollar call (in the output()
	     * call), "num" holds the number associated with N, or DOLLAR_DOLLAR
	     * in the case of $$.
	     */

	    if( *++action != '<' )
		*fname = '\0';
	    else
	    {
		++action;	/* skip the < */
		fp = fname;

		for(i=sizeof(fname); --i>0 && *action && *action != '>'; )
		    *fp++ = *action++;

		*fp = '\0';
		if( *action == '>' )
		    ++action;
	    }

	    if( *action == '$' )
	    {
		num = DOLLAR_DOLLAR;
		++action;
	    }
	    else
	    {
		num = atoi( action );
		if( *action == '-' )
			++action ;
		while( isdigit(*action) )
			++action ;
	    }

	    output( "%s", do_dollar( num, rhs_size, lineno, prod, fname ));
	}
    }
    output("\n        break;\n"		 );
}
