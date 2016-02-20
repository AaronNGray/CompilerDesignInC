/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/compiler.h>
#include "nfa.h"			/* defines for NFA, EPSILON, CCL */
#include "dfa.h"			/* defines for DFA_STATE, etc.	 */
/*----------------------------------------------------------------------*/
#define LARGEST_INT  (int)(((unsigned)(~0)) >> 1)

PRIVATE	NFA  *Nfa;			/* Base address of NFA array	*/
PRIVATE	int  Nfa_states;    		/* Number of states in NFA	*/
/*----------------------------------------------------------------------*/
PUBLIC int nfa( input_routine )
char	*(*input_routine)P((void));
{
    /* Compile the NFA and initialize the various global variables used by
     * move() and e_closure(). Return the state number (index) of the NFA start
     * state. This routine must be called before either e_closure() or move()
     * are called. The memory used for the nfa can be freed with free_nfa()
     * (in thompson.c).
     */

    NFA	*sstate;
    Nfa = thompson(input_routine, &Nfa_states, &sstate);
    return( sstate - Nfa );
}
/*----------------------------------------------------------------------*/
PUBLIC void free_nfa()
{
    free( Nfa );
}
PUBLIC  SET  *e_closure( input, accept, anchor )
SET	*input   ;
char	**accept ;
int	*anchor  ;
{
    /* input    is the set of start states to examine.
     * *accept  is modified to point at the string associated with an accepting
     *	        state (or to NULL if the state isn't an accepting state).
     * *anchor  is modified to hold the anchor point, if any.
     *
     * Computes the epsilon closure set for the input states. The output set
     * will contain all states that can be reached by making epsilon transitions
     * from all NFA states in the input set. Returns an empty set if the input
     * set or the closure set is empty, modifies *accept to point at the
     * accepting string if one of the elements of the output state is an
     * accepting state.
     */

    int  stack[ NFA_MAX ]; 		/* Stack of untested states	*/
    int  *tos; 	    	   		/* Stack pointer		*/
    NFA  *p;		   		/* NFA state being examined	*/
    int  i;				/* State number of "		*/
    int  accept_num = LARGEST_INT ;

     D( printf("e_closure of {"); 	)
     D( pset(input, fprintf, stdout);	)
     D( printf("}="); 			)

    if( !input )
	    goto abort;

      /*	1:  push all states in input set onto the stack
       *	2:  while( the stack is not empty )
       *	3:      pop the top element i and, if it's an
       *		   accept state, *accept = the accept string;
       *	4:	if( there's an epsilon transition from i to u )
       *	5:	   if( u isn't in the closure set )
       *	6:		add u to the closure set
       *	7:		push u onto the stack
       */

    *accept = NULL;			          /* Reference to algorithm: */
    tos     = & stack[-1];					/* 1 */

    for( next_member(NULL); (i = next_member(input)) >= 0 ;)
	*++tos = i;

    while( INBOUNDS(stack,tos) )				/* 2 */
    {
	i = *tos-- ;						/* 3 */
	p = & Nfa[ i ];
	if( p->accept && (i < accept_num) )
	{
	    accept_num = i ;
	    *accept    = p->accept ;
	    *anchor    = p->anchor ;
	}

	if( p->edge == EPSILON )				/* 4 */
	{
	    if( p->next )
	    {
		i = p->next - Nfa;
		if( !MEMBER(input, i) ) 			/* 5  */
		{
		    ADD( input, i );				/* 6  */
		    *++tos = i;					/* 7  */
		}
	    }
	    if( p->next2 )
	    {
		i = p->next2 - Nfa;
		if( !MEMBER(input, i) )				/* 5  */
		{
		    ADD( input, i );				/* 6  */
		    *++tos = i;					/* 7  */
		}
	    }
	}
    }
abort:

    D( printf("{"); 						)	  /*} */
    D( pset(input, fprintf, stdout );				)	  /*{{*/
    D( printf(*accept ? "} ACCEPTING<%s>\n" : "}\n", *accept);	)

    return input;
}
PUBLIC  SET *move( inp_set, c )
SET	*inp_set;			/* input set			*/
int	c;				/* transition on this character */
{
    /* Return a set that contains all NFA states that can be reached by making
     * transitions on "c" from any NFA state in "inp_set." Returns NULL if
     * there are no such transitions. The inp_set is not modified.
     */

    int  i;
    NFA  *p;					/* current NFA state 	*/
    SET  *outset = NULL;			/* output set		*/

    D(  printf ("move({"); 			)
    D(  pset   (inp_set, fprintf, stdout);	)
    D(  printf ("}, ");				)
    D(  pchar  (c, stdout );			)
    D(  printf ( ")=" );			)

    for( i = Nfa_states; --i >= 0; )
    {
	if( MEMBER(inp_set, i) )
	{
	    p = &Nfa[i];

	    if( p->edge==c || (p->edge==CCL && TEST(p->bitset, c)))
	    {
		if( !outset )
		    outset = newset();

		ADD( outset, p->next - Nfa );
	    }
	}
    }
    D( pset(outset, fprintf, stdout);	)
    D( printf("\n");			)

    return( outset );
}
#ifdef MAIN
#define ALLOCATE
#include "globals.h"	/* externs for Verbose */

#define BSIZE	256

PRIVATE char	Buf[ BSIZE ];	/* input buffer				*/
PRIVATE char	*Pbuf = Buf;	/* current position in input buffer	*/
PRIVATE char	*Expr;		/* regular expression from command line */

int	nextchar( ANSI(void) )
{
    if( !*Pbuf )
    {
	if( !fgets(Buf, BSIZE, stdin) )
	    return NULL;
	Pbuf = Buf;
    }
    return *Pbuf++;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PRIVATE void	printbuf( ANSI(void) )
{
    fputs(Buf, stdout);		/* Print the buffer and force a read	*/
    *Pbuf = 0;			/* on the next call to nextchar().	*/
}
/*--------------------------------------------------------------*/
PRIVATE char	*getline( ANSI(void) )
{
    static int	first_time_called = 1;
    if( !first_time_called )
	return NULL;

    first_time_called = 0;
    return Expr;
}
/*--------------------------------------------------------------*/

void main( argc, argv )
int argc;	//xxx
char **argv;
{
    int	sstate;		 /* Starting NFA state		*/
    SET	*start_dfastate; /* Set of starting nfa states  */
    SET	*current;	 /* current DFA state		*/
    SET	*next;
    char *accept;	 /* cur. DFA state is an accept	*/
    int	c;		 /* current input character	*/
    int anchor;

    if( argc == 2 )
	fprintf(stderr,"expression is %s\n", argv[1] );
    else
    {
	fprintf(stderr,"usage: terp pattern <input\n");
	exit(1);
    }

    /*  1: Compile the NFA; initialize move() & e_closure().
     *  2: Create the initial state, the set of all NFA states that can
     *	   be reached by making epsilon transitions from the NFA start state.
     *	   Note that e_closure() returns the original set with elements
     *	   added to it as necessary.
     *  3: Initialize the current state to the start state.
     */

    Expr   = argv[1];					/* 1 */
    sstate = nfa( getline );

    start_dfastate = newset();				/* 2 */
    ADD( start_dfastate, sstate );
    if( !e_closure( start_dfastate, &accept, &anchor) )
    {
	fprintf(stderr, "Internal error: State machine is empty\n");
	exit(1);
    }

    current = newset();					/* 3 */
    ASSIGN( current, start_dfastate );

    /* Now interpret the NFA: The next state is the set of all NFA states that
     * can be reached after we've made a transition on the current input
     * character from any of the NFA states in the current state. The current
     * input line is printed every time an accept state is encountered.
     * The machine is reset to the initial state when a failure transition is
     * encountered.
     */

    while( c = nextchar() )
    {
	next = e_closure( move(current, c), &accept, &anchor );
	if( accept )
	{
	    printbuf();				/* accept	*/
	    if( next );
		delset( next );			/* reset	*/
	    ASSIGN( current, start_dfastate );
	}
	else
	{					/* keep looking	*/
	    delset( current );
	    current = next;
	}
    }
    delset( current	   );	/* Not required for main, but you'll */
    delset( start_dfastate );	/* need it when adapting main() to a */
}				/* subroutine.			     */
#endif
