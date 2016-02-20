/*@A (C) 1992 Allen I. Holub                                                */

/* NFA.C---Make an NFA from a LeX input file using Thompson's construction */

#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/stack.h>
#include "nfa.h"			/* defines for NFA, EPSILON, CCL */
#include "dfa.h"			/* prototype for lerror().	 */
#include "globals.h"			/* externs for Verbose, etc.	 */

#ifdef DEBUG
	int    Lev = 0;
#	define ENTER(f) printf("%*senter %s [%c][%1.10s] \n",  		   \
					    Lev++ * 4, "", f, Lexeme, Input)
#	define LEAVE(f) printf("%*sleave %s [%c][%1.10s]\n",   		   \
					    --Lev * 4, "", f, Lexeme, Input)
#else
#	define ENTER(f)
#	define LEAVE(f)
#endif
/*----------------------------------------------------------------
 *    Error processing stuff. Note that all errors are fatal.
 *----------------------------------------------------------------
 */

typedef enum ERR_NUM
{
    E_MEM,	/* Out of memory				*/
    E_BADEXPR,	/* Malformed regular expression			*/
    E_PAREN,	/* Missing close parenthesis			*/
    E_STACK,	/* Internal error: Discard stack full		*/
    E_LENGTH,	/* Too many regular expressions 		*/
    E_BRACKET,	/* Missing [ in character class			*/
    E_BOL,	/* ^ must be at start of expr or ccl		*/
    E_CLOSE,	/* + ? or * must follow expression		*/
    E_STRINGS,	/* Too many characters in accept actions	*/
    E_NEWLINE,	/* Newline in quoted string			*/
    E_BADMAC,	/* Missing } in macro expansion			*/
    E_NOMAC,	/* Macro doesn't exist				*/
    E_MACDEPTH  /* Macro expansions nested too deeply.		*/

} ERR_NUM;


PRIVATE char	*Errmsgs[] =		/* Indexed by ERR_NUM */
{
    "Not enough memory for NFA",
    "Malformed regular expression",
    "Missing close parenthesis",
    "Internal error: Discard stack full",
    "Too many regular expressions or expression too long",
    "Missing [ in character class",
    "^ must be at start of expression or after [",
    "+ ? or * must follow an expression or subexpression",
    "Too many characters in accept actions",
    "Newline in quoted string, use \\n to get newline into expression",
    "Missing } in macro expansion",
    "Macro doesn't exist",
    "Macro expansions nested too deeply"
};

typedef enum WARN_NUM
{
    W_STARTDASH,	/* Dash at start of character class	*/
    W_ENDDASH  		 /* Dash at end   of character class	*/

} WARN_NUM;


PRIVATE char	*Warnmsgs[] =		/* Indexed by WARN_NUM */
{
    "Treating dash in [-...] as a literal dash",
    "Treating dash in [...-] as a literal dash"
};


PRIVATE NFA   *Nfa_states  ; 	     /* State-machine array		   */
PRIVATE int   Nstates = 0  ;  	     /* # of NFA states in machine	   */
PRIVATE int   Next_alloc;  	     /* Index of next element of the array */

#define	SSIZE	32

PRIVATE NFA   *Sstack[ SSIZE ];	     /* Stack used by new()		   */
PRIVATE NFA   **Sp = &Sstack[ -1 ];  /* Stack pointer			   */

#define	STACK_OK()    ( INBOUNDS(Sstack, Sp) )     /* true if stack not  */
						   /* full or empty	 */
#define STACK_USED()  ( (int)(Sp-Sstack) + 1	)  /* slots used         */
#define CLEAR_STACK() ( Sp = Sstack - 1		)  /* reset the stack    */
#define PUSH(x)	      ( *++Sp = (x)		)  /* put x on stack     */
#define POP()	      ( *Sp--			)  /* get x from stack   */


/*--------------------------------------------------------------
 * MACRO support:
 */

#define MAC_NAME_MAX  34	    /* Maximum name length		*/
#define MAC_TEXT_MAX  80	    /* Maximum amount of expansion text	*/

typedef struct MACRO
{
    char name[ MAC_NAME_MAX ];
    char text[ MAC_TEXT_MAX ];

} MACRO;

PRIVATE HASH_TAB *Macros; /* Symbol table for macro definitions */
typedef enum TOKEN
{
    EOS = 1,		/*  end of string	*/
    ANY,		/*  .		 	*/
    AT_BOL,		/*  ^			*/
    AT_EOL,		/*  $			*/
    CCL_END,		/*  ]			*/
    CCL_START,		/*  [			*/
    CLOSE_CURLY,	/*  }			*/
    CLOSE_PAREN,	/*  )			*/
    CLOSURE,		/*  *			*/
    DASH,		/*  -			*/
    END_OF_INPUT,	/*  EOF  		*/
    L,			/*  literal character	*/
    OPEN_CURLY,		/*  {			*/
    OPEN_PAREN,		/*  (			*/
    OPTIONAL,		/*  ?			*/
    OR,			/*  |			*/
    PLUS_CLOSE		/*  +			*/

} TOKEN;


PRIVATE TOKEN	Tokmap[] =
{
/*  ^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N	*/
     L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  ^O  ^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\  ^]	*/
     L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  ^^  ^_  SPACE  !   "   #    $        %   &    '		*/
    L,  L,  L,     L,  L,  L,   AT_EOL,  L,  L,   L,

/*  (		 )            *	       +           ,  -     .   */
    OPEN_PAREN,  CLOSE_PAREN, CLOSURE, PLUS_CLOSE, L, DASH, ANY,

/*  /   0   1   2   3   4   5   6   7   8   9   :   ;   <   =	*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  >         ?							*/
    L,        OPTIONAL,

/*  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N	*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  O   P   Q   R   S   T   U   V   W   X   Y   Z   		*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  [		\	]		^			*/
    CCL_START,	L,	CCL_END, 	AT_BOL,

/*  _   `   a   b   c   d   e   f   g   h   i   j   k   l   m	*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  n   o   p   q   r   s   t   u   v   w   x   y   z   	*/
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,

/*  {            |    }            DEL 				*/
    OPEN_CURLY,  OR,  CLOSE_CURLY, L
};

PRIVATE char  *(*Ifunct) P((void)) ; /* Input function pointer		 */
PRIVATE char  *Input  = "" ;	     /* Current position in input string */
PRIVATE char  *S_input 	   ;  	     /* Beginning of input string	 */
PRIVATE TOKEN Current_tok  ;  	     /* Current token		 	 */
PRIVATE int   Lexeme	   ;  	     /* Value associated with LITERAL	 */

#define	MATCH(t)   (Current_tok == (t))

PRIVATE void 	parse_err	P(( ERR_NUM type		));
PRIVATE	void	warning		P(( WARN_NUM type		));
PRIVATE void	errmsg		P(( int type, char **table, char *msgtype ));
PRIVATE NFA 	*new		P(( void 			));
PRIVATE void	discard		P(( NFA *nfa_to_discard		));
PRIVATE char	*save		P(( char *str			));
PRIVATE char	*get_macro	P(( char **namep		));
PRIVATE void	print_a_macro	P(( MACRO *mac			));
PRIVATE TOKEN	advance		P(( void 			));
PRIVATE NFA	*machine	P(( void 			));
PRIVATE NFA	*rule		P(( void 			));
PRIVATE void	expr		P(( NFA **startp, NFA **endp	));
PRIVATE void	cat_expr	P(( NFA **startp, NFA **endp	));
PRIVATE int	first_in_cat	P(( TOKEN tok			));
PRIVATE void	factor		P(( NFA **startp, NFA **endp	));
PRIVATE void	term		P(( NFA **startp, NFA **endp	));
PRIVATE void	dodash		P(( SET *set			));



PRIVATE	void	warning( type )
WARN_NUM	type;			/* Print error mesage and arrow to */
{
    errmsg( (int)type, Warnmsgs, "WARNING" );
}

PRIVATE	void	parse_err( type )
ERR_NUM	type;				/* Print error mesage and arrow to */
{
    errmsg( (int)type, Errmsgs, "ERROR" );
    exit(1);
}

PRIVATE void errmsg( type, table, msgtype )
int  type;
char **table;
char *msgtype;
{			  	  	/* highlight point of error.       */
    char *p;
    fprintf(stderr, "%s (line %d) %s\n", msgtype, Actual_lineno, table[type] );

    for( p = S_input; ++p <= Input; putc('-', stderr) )
	;
    fprintf( stderr, "v\n%s\n", S_input );
    exit( 1 );
}

/*--------------------------------------------------------------*/

PRIVATE	NFA  *new()			/* NFA management functions */
{
    NFA	*p;
    static int first_time = 1;

    if( first_time )
    {
	if( !( Nfa_states = (NFA *) calloc(NFA_MAX, sizeof(NFA)) ))
	    parse_err( E_MEM );	/* doesn't return */

	first_time = 0;
	Sp = &Sstack[ -1 ];
    }

    if( ++Nstates >= NFA_MAX )
	parse_err( E_LENGTH );	/* doesn't return */

    /* If the stack is not ok, it's empty */

    p = !STACK_OK() ? &Nfa_states[Next_alloc++] : POP();
    p->edge = EPSILON;
    return p;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

PRIVATE	void	discard( nfa_to_discard )
NFA	*nfa_to_discard;
{
    --Nstates;

    memset( nfa_to_discard, 0, sizeof(NFA) );
    nfa_to_discard->edge = EMPTY ;
    PUSH( nfa_to_discard );

    if( !STACK_OK() )
	parse_err( E_STACK );	/* doesn't return */
}

/*----------------------------------------------------------------------*/

PRIVATE char	*save( str )		/* String-management function. */
char	*str;
{
    char	*textp, *startp;
    int		len;
    static int	first_time = 1;
    static char size[8];	/* Query-mode size */
    static int  *strings;  	 /* Place to save accepting strings	*/
    static int  *savep;		 /* Current position in strings array.	*/

    if( first_time )
    {
	if( !(savep = strings = (int *) malloc( STR_MAX )) )
	    parse_err( E_MEM );	/* doesn't return */
	first_time = 0;
    }

    if( !str ) /* query mode. Return number of bytes in use */
    {
	sprintf( size, "%ld", (long)(savep - strings) );
	return size;
    }

    if( *str == '|' )
	return (char *)( savep + 1 );

    *savep++ = Lineno;

    for( textp = (char *)savep ; *str ; *textp++ = *str++ )
	if( textp >=  (char *)strings + (STR_MAX-1) )
	    parse_err( E_STRINGS );	/* doesn't return */

    *textp++ = '\0' ;

    /* Increment savep past the text. "len" is initialized to the string length.
     * The "len/sizeof(int)" truncates the size down to an even multiple of the
     * current int size. The "+(len % sizeof(int) != 0)" adds 1 to the truncated
     * size if the string length isn't an even multiple of the int size (the !=
     * operator evaluates to 1 or 0). Return a pointer to the string itself.
     */

    startp  = (char *)savep;
    len     = textp - startp;
    savep  += (len / sizeof(int)) + (len % sizeof(int) != 0);
    return startp;
}

/*------------------------------------------------------------*/

PUBLIC  void	new_macro( def )
char	*def;
{
    /* Add a new macro to the table. If two macros have the same name, the
     * second one takes precedence. A definition takes the form:
     *		 name <whitespace> text [<whitespace>]
     * whitespace at the end of the line is ignored.
     */

    unsigned hash_add();

    char  *name;		/* Name component of macro definition	*/
    char  *text;		/* text part of macro definition	*/
    char  *edef;		/* pointer to end of text part		*/
    MACRO *p;
    static int first_time = 1;

    if( first_time )
    {
	first_time = 0;
	Macros = maketab( 31, hash_add, strcmp );
    }

    for( name = def; *def && !isspace(*def) ; def++ )	   /* Isolate name */
	;
    if( *def )
	*def++ = '\0' ;

    /* Isolate the definition text. This process is complicated because you need
     * to discard any trailing whitespace on the line. The first while loop
     * skips the preceding whitespace. The for loop is looking for end of
     * string. If you find a white character (and the \n at the end of string
     * is white), remember the position as a potential end of string.
     */

    while( isspace( *def ) )	     /* skip up to macro body 		   */
	++def;

    text = def;			     /* Remember start of replacement text */

    edef = NULL;		     /* strip trailing white space	   */
    while(  *def  )
    {
	if( !isspace(*def) )
	    ++def;
	else
	    for(edef = def++; isspace(*def) ; ++def )
		;
    }

    if( edef )
	*edef = '\0';
				     /* Add the macro to the symbol table  */

    p  = (MACRO *) newsym( sizeof(MACRO) );
    strncpy( p->name, name, MAC_NAME_MAX );
    strncpy( p->text, text, MAC_TEXT_MAX );
    addsym( Macros, p );

    D( printf("Added macro definition, macro table now is:\n");	)
    D( print_macs();						)
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE char	*get_macro( namep )
char	**namep;
{
    /* Return a pointer to the contents of a macro having the indicated
     * name. Abort with a message if no macro exists. The macro name includes
     * the brackets. *namep is modified to point past the close brace.
     */

    char   *p;
    MACRO  *mac;
						/* {		     */
    if( !(p = strchr( ++(*namep), '}')) )	/* skip { and find } */
	parse_err( E_BADMAC );			/* print msg & abort */
    else
    {
	*p = '\0';				/* Overwrite close brace. { */
	if( !(mac = (MACRO *) findsym( Macros, *namep )) )
	    parse_err( E_NOMAC );
	*p++   = '}';				/* Put the brace back.	  */

	*namep =  p ;				/* Update name pointer.   */
	return mac->text;
    }

    return "ERROR";			    /* If you get here, it's a bug */
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE	void print_a_macro( mac )	/* Workhorse function needed by      */
MACRO	*mac;				/* ptab() call in printmacs(), below */
{
    printf( "%-16s--[%s]--\n", mac->name, mac->text );
}

PUBLIC void printmacs( ANSI(void) )	/* Print all the macros to stdout */
{
    if( !Macros )
	printf("\tThere are no macros\n");
    else
    {
	printf("\nMACROS:\n");
	ptab( Macros, (void(*)P((void*,...))) print_a_macro, NULL, 1 );
    }
}

/*----------------------------------------------------------------
 * Lexical analyzer:
 *
 * Lexical analysis is trivial because all lexemes are single-character values.
 * The only complications are escape sequences and quoted strings, both
 * of which are handled by advance(), below. This routine advances past the
 * current token, putting the new token into Current_tok and the equivalent
 * lexeme into Lexeme. If the character was escaped, Lexeme holds the actual
 * value. For example, if a "\s" is encountered, Lexeme will hold a space
 * character.  The MATCH(x) macro returns true if x matches the current token.
 * Advance both modifies Current_tok to the current token and returns it.
 *
 * Macro expansion is handled by means of a stack (declared at the top
 * of the subroutine). When an expansion request is encountered, the
 * current input buffer is stacked, and input is read from the macro
 * text. This process repeats with nested macros, so SSIZE controls
 * the maximum macro-nesting depth.
 */

PRIVATE	TOKEN	advance()
{
    static int   inquote = 0;   	/* Processing quoted string	*/
    int	         saw_esc;       	/* Saw a backslash		*/
    static char  *stack[ SSIZE ],	/* Input-source stack		*/
		 **sp = NULL;		/* and stack pointer.		*/

    if( !sp )				/* Initialize sp. 		*/
        sp = stack - 1;			/* Necessary for large model    */

    if( Current_tok == EOS )		/* Get another line	        */
    {
	if( inquote )
	    parse_err( E_NEWLINE );	/* doesn't return */
	do
	{
	    /* Sit in this loop until a non-blank line is read into	*/
	    /* the "Input" array.					*/

	    if( !(Input = (*Ifunct)()) )	/* then at end of file  */
	    {
		Current_tok = END_OF_INPUT;
		goto exit;
	    }
	    while( isspace( *Input ) )	  	/* Ignore leading	  */
		Input++;		  	/* white space...	  */

	} while ( !*Input );		  	/* and blank lines.	  */

	S_input = Input;		  	/* Remember start of line */
    }				          	/* for error messages.    */

    while( *Input == '\0' )
    {
	if( INBOUNDS(stack, sp) )	 /* Restore previous input source */
	{
	    Input = *sp-- ;
	    continue;
	}

	Current_tok = EOS;	       /* No more input sources to restore */
	Lexeme = '\0';		       /* ie. you're at the real end of    */
	goto exit;		       /* string.			   */
    }

    if( !inquote )
    {
	while( *Input == '{' )		  /* Macro expansion required     }*/
	{
	    *++sp = Input ;		  /* Stack current input string.    */
	    Input = get_macro( sp );	  /* Use macro body as input string */

	    if( TOOHIGH(stack,sp) )
		parse_err(E_MACDEPTH);	  /* Stack overflow, abort */
	}
    }

    if( *Input == '"' )
    {				  /* At either start and end of a quoted      */
	inquote = ~inquote;	  /* string. All characters are treated  as   */
	if( !*++Input )		  /* literals while inquote is true).         */
	{
	    Current_tok = EOS ;
	    Lexeme = '\0';
	    goto exit;
	}
    }

    saw_esc = (*Input == '\\');

    if( !inquote )
    {
	if( isspace(*Input) )
	{
	    Current_tok = EOS ;
	    Lexeme	= '\0';
	    goto exit;
	}
	Lexeme = esc( &Input );
    }
    else
    {
	if( saw_esc && Input[1] == '"' )
	{
	    Input  += 2;	/* Skip the escaped character */
	    Lexeme = '"';
	}
	else
	    Lexeme = *Input++ ;
    }

    Current_tok = (inquote || saw_esc) ? L : Tokmap[Lexeme] ;
exit:
    return Current_tok;
}


/*--------------------------------------------------------------
 * The Parser:
 *	A simple recursive descent parser that creates a Thompson NFA for
 * 	a regular expression. The access routine [thompson()] is at the
 *	bottom. The NFA is created as a directed graph, with each node
 *	containing pointer's to the next node. Since the structures are
 *	allocated from an array, the machine can also be considered
 *	as an array where the state number is the array index.
 */

PRIVATE  NFA	*machine()
{
    NFA	*start;
    NFA	*p;

    ENTER("machine");

    p = start = new();
    p->next   = rule();

    while( !MATCH(END_OF_INPUT) )
    {
	p->next2 = new();
	p        = p->next2;
	p->next  = rule();
    }

    LEAVE("machine");
    return start;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE NFA  *rule()
{
    /*	rule	--> expr  EOS action
     *		    ^expr EOS action
     *		    expr$ EOS action
     *
     *	action	--> <tabs> <string of characters>
     *		    epsilon
     */

    NFA	*start = NULL;
    NFA	*end   = NULL;
    int	anchor = NONE;

    ENTER("rule");

    if( MATCH( AT_BOL ) )
    {
    	start 	    =  new() ;
	start->edge =  '\n'  ;
	anchor      |= START ;
	advance();
	expr( &start->next, &end );
    }
    else
	expr( &start, &end );

    if( MATCH( AT_EOL ) )
    {
	/*  pattern followed by a carriage-return or linefeed (use a
	 *  character class).
	 */

	advance();
    	end->next =  new()     ;
	end->edge =  CCL       ;

	if( !( end->bitset = newset()) )
	    parse_err( E_MEM );		/* doesn't return */

	ADD( end->bitset, '\n' );

	if( !Unix )
	    ADD( end->bitset, '\r' );

	end       =  end->next ;
	anchor    |= END       ;
    }

    while( isspace(*Input) )
	Input++ ;

    end->accept = save( Input );
    end->anchor = anchor;
    advance();				/* skip past EOS */

    LEAVE("rule");
    return start;
}

PRIVATE	void expr( startp,  endp )
NFA	**startp, **endp ;
{
    /* Because a recursive descent compiler can't handle left recursion,
     * the productions:
     *
     *	expr	-> expr OR cat_expr
     *		|  cat_expr
     *
     * must be translated into:
     *
     *	expr	-> cat_expr expr'
     *	expr'	-> OR cat_expr expr'
     *		   epsilon
     *
     * which can be implemented with this loop:
     *
     *	cat_expr
     *	while( match(OR) )
     *		cat_expr
     *		do the OR
     */

    NFA	*e2_start = NULL; /* expression to right of | */
    NFA *e2_end   = NULL;
    NFA *p;

    ENTER("expr");

    cat_expr( startp, endp );

    while( MATCH( OR ) )
    {
	advance();
	cat_expr( &e2_start, &e2_end );

	p = new();
	p->next2 = e2_start ;
	p->next  = *startp  ;
	*startp  = p;

	p = new();
	(*endp)->next = p;
	e2_end ->next = p;
	*endp = p;
    }
    LEAVE("expr");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE	void cat_expr( startp,  endp )
NFA	**startp, **endp ;
{
    /* The same translations that were needed in the expr rules are needed again
     * here:
     *
     *	cat_expr  -> cat_expr | factor
     *		     factor
     *
     * is translated to:
     *
     *	cat_expr  -> factor cat_expr'
     *	cat_expr' -> | factor cat_expr'
     *		     epsilon
     */

    NFA	 *e2_start, *e2_end;

    ENTER("cat_expr");

    if( first_in_cat( Current_tok ) )
	factor( startp, endp );

    while(  first_in_cat( Current_tok )  )
    {
	factor( &e2_start, &e2_end );

	memcpy( *endp, e2_start, sizeof(NFA));
	discard( e2_start );

	*endp = e2_end;
    }

    LEAVE("cat_expr");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE	int	first_in_cat( tok )
TOKEN	tok;
{
    switch( tok )
    {
    case CLOSE_PAREN:
    case AT_EOL:
    case OR:
    case EOS:		return 0;


    case CLOSURE:
    case PLUS_CLOSE:
    case OPTIONAL:	parse_err( E_CLOSE   );	return 0;

    case CCL_END:	parse_err( E_BRACKET );	return 0;
    case AT_BOL:	parse_err( E_BOL     );	return 0;
    }

    return 1;
}

PRIVATE	void factor( startp, endp )
NFA	**startp, **endp;
{
     /*		factor	--> term*  | term+  | term?
     */

    NFA	*start, *end;

    ENTER("factor");

    term( startp, endp );

    if( MATCH(CLOSURE) || MATCH(PLUS_CLOSE) || MATCH(OPTIONAL) )
    {
	start 	  = new()   ;
	end   	  = new()   ;
	start->next   = *startp ;
	(*endp)->next = end     ;

	if( MATCH(CLOSURE) || MATCH(OPTIONAL) )		  /*   * or ?   */
	    start->next2 = end;

	if( MATCH(CLOSURE) || MATCH(PLUS_CLOSE) )	  /*   * or +   */
	    (*endp)->next2 = *startp;

	*startp  = start ;
	*endp    = end   ;
	advance();
    }

    LEAVE("factor");
}

PRIVATE	void term( startp, endp )
NFA	**startp, **endp;
{
    /* Process the term productions:
     *
     * term  --> [...]  |  [^...]  |  []  |  [^] |  .  | (expr) | <character>
     *
     * The [] is nonstandard. It matches a space, tab, formfeed, or newline,
     * but not a carriage return (\r). All of these are single nodes in the
     * NFA.
     */

    NFA	*start;
    int c;

    ENTER("term");

    if( MATCH( OPEN_PAREN ) )
    {
	advance();
	expr( startp, endp );
	if( MATCH( CLOSE_PAREN ) )
	    advance();
	else
	    parse_err( E_PAREN );	/* doesn't return */
    }
    else
    {
	*startp = start 	  = new();
	*endp   = start->next = new();

	if( !( MATCH( ANY ) || MATCH( CCL_START) ))
	{
	    start->edge = Lexeme;
	    advance();
	}
	else
	{
	    start->edge = CCL;

	    if( !( start->bitset = newset()) )
		parse_err( E_MEM );		/* doesn't return */

	    if( MATCH( ANY ) )			/* 	dot (.)	   */
	    {
		ADD( start->bitset, '\n' );
		if( !Unix )
		    ADD( start->bitset, '\r' );

		COMPLEMENT( start->bitset );
	    }
	    else
	    {
		advance();
		if( MATCH( AT_BOL ) )		/* Negative character class */
		{
		    advance();

		    ADD( start->bitset, '\n' );	/* Don't include \n in class */
		    if( !Unix )
			ADD( start->bitset, '\r' );

		    COMPLEMENT( start->bitset );
		}
		if( ! MATCH( CCL_END ) )
		    dodash( start->bitset );
		else				/* 	[] or [^]	    */
		    for( c = 0; c <= ' '; ++c )
			ADD( start->bitset, c );
	    }
	    advance();
	}
    }
    LEAVE("term");
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

PRIVATE	void	dodash( set )
SET	*set;				/* Pointer to ccl character set	*/
{
    register int	first;

    if( MATCH( DASH ) )		/* Treat [-...] as a literal dash */
    {				/* But print warning.		  */
	warning( W_STARTDASH );
	ADD( set, Lexeme );
	advance();
    }

    for(; !MATCH( EOS )  &&  !MATCH( CCL_END ) ; advance() )
    {
	if( ! MATCH( DASH ) )
	{
	    first = Lexeme;
	    ADD( set, Lexeme );
	}
	else	/* looking at a dash */
	{
	    advance();
	    if( MATCH( CCL_END ) )	/* Treat [...-] as literal */
	    {
		warning( W_ENDDASH );
		ADD( set, '-' );
	    }
	    else
	    {
		for(; first <= Lexeme ; first++ )
		    ADD( set, first );
	    }
	}
    }
}


PUBLIC  NFA	*thompson( input_function, max_state, start_state )
char	*(*input_function) P((void));
int	*max_state;
NFA	**start_state;
{
    /* Access routine to this module. Return a pointer to a NFA transition
     * table that represents the regular expression pointed to by expr or
     * NULL if there's not enough memory. Modify *max_state to reflect the
     * largest state number used. This number will probably be a larger
     * number than the total number of states. Modify *start_state to point
     * to the start state. This pointer is garbage if thompson() returned 0.
     * The memory for the table is fetched from malloc(); use free() to
     * discard it.
     */

    CLEAR_STACK();

    Ifunct = input_function;

    Current_tok = EOS;		/* Load first token	*/
    advance();

    Nstates    = 0;
    Next_alloc = 0;

    *start_state = machine();	/* Manufacture the NFA	*/
    *max_state   = Next_alloc ;	/* Max state # in NFA   */

    if( Verbose > 1 )
	print_nfa( Nfa_states, *max_state, *start_state );

    if( Verbose )
    {
	printf("%d/%d NFA states used.\n", *max_state, NFA_MAX );
	printf("%s/%d bytes used for accept strings.\n\n", save(NULL), STR_MAX);
    }
    return Nfa_states;
}



#ifdef MAIN

#define ALLOC
#include "globals.h"

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

PRIVATE pnode( nfa )	/* For debugging, print a single NFA structure */
NFA	*nfa;
{
    if( !nfa )
	    printf("(NULL)");
    else
    {
	printf( "+--Structure at 0x%04x-------------------\n", nfa );
	printf( "|edge   = 0x%x (%c)\n", nfa->edge, nfa->edge );
	printf( "|next   = 0x%x\n", nfa->next );
	printf( "|next2  = 0x%x\n", nfa->next2 );
	printf( "|accept = <%s>\n", nfa->accept );
	printf( "|bitset = " );
	printccl( nfa->bitset );
	printf("\n");
	printf( "+----------------------------------------\n" );
    }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

PRIVATE char	*getline()
{
    static char	buf[80];

    printf("%d: ", Lineno++ );
    gets( buf );

    return  *buf ? buf : NULL ;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

main( argc, argv )
char	**argv;
{
    NFA	*nfa, *start_state;
    int	max_state;

    Verbose = 2;

    nfa = thompson(getline, &max_state, &start_state);
}

#endif
