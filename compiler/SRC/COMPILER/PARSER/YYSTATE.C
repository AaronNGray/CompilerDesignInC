/*@A (C) 1992 Allen I. Holub                                                */
  /* YYSTATE.C	Routines to manufacture the lr(1) state table.  */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "parser.h"
#include "llout.h"			/* For _EOI_ definition */

/*----------------------------------------------------------------------*/
					/* For statistics only:		     */
PRIVATE int	Nitems		= 0;	/* number of LR(1) items	     */
PRIVATE int	Npairs		= 0;	/* # of pairs in output tables	     */
PRIVATE int	Ntab_entries	= 0;	/* number of transitions in tables    */
PRIVATE int	Shift_reduce    = 0;	/* number of shift/reduce conflicts  */
PRIVATE int	Reduce_reduce   = 0;	/* number of reduce/reduce conflicts */

#define MAXSTATE   512    /* Max # of LALR(1) states.			*/
#define MAXOBUF	   256    /* Buffer size for various output routines	*/

/*----------------------------------------------------------------------*/

typedef struct _item_		    /* LR(1) item:		   	      */
{
   int		 prod_num;	    /* production number 		      */
   PRODUCTION 	 *prod;		    /* the production itself 		      */
   SYMBOL	 *right_of_dot;     /* symbol to the right of the dot	      */
   unsigned char dot_posn;	    /* offset of dot from start of production */
   SET		 *lookaheads;	    /* set of lookahead symbols for this item */

} ITEM;


#define RIGHT_OF_DOT(p) ( (p)->right_of_dot ? (p)->right_of_dot->val : 0 )

#define MAXKERNEL  32	/* Maximum number of kernel items in a state.	      */
#define MAXCLOSE   128	/* Maximum number of closure items in a state (less   */
			/* the epsilon productions).			      */
#define MAXEPSILON 8	/* Maximum number of epsilon productions that can be  */
			/* in a closure set for any given state.	      */

typedef short STATENUM;


typedef struct _state_			/* LR(1) state			    */
{
   ITEM	 *kernel_items  [MAXKERNEL ];	/* Set of kernel items.		    */
   ITEM	 *epsilon_items [MAXEPSILON];	/* Set of epsilon items.	    */

   unsigned nkitems  : 7 ;		/* # items in kernel_items[].	    */
   unsigned neitems  : 7 ;		/* # items in epsilon_items[].	    */
   unsigned closed   : 1 ;		/* State has had closure performed. */

   STATENUM num;			/* State number (0 is start state). */

} STATE;


typedef struct act_or_goto
{
    int	sym;			/* Given this input symbol, 		   */
    int do_this;		/* do this. >0 == shift, <0 == reduce 	   */
    struct act_or_goto *next;	/* Pointer to next ACT in the linked list. */

} ACT;

typedef ACT GOTO;		/* GOTO is an alias for ACT */

PRIVATE ACT *Actions[MAXSTATE];	/* Array of pointers to the head of the action
			         * chains. Indexed by state number.
			 	 * I'm counting on initialization to NULL here.
			         */
PRIVATE GOTO *Gotos[MAXSTATE]; 	/* Array of pointers to the head of the goto
			 	 * chains.
			 	 */
#define CHUNK	   128		 /* New() gets this many structures at once */
PRIVATE HASH_TAB *States      = NULL;	/* LR(1) states 		 */
PRIVATE int	 Nstates      = 0;	/* Number of states.	 	 */

#define MAX_UNFINISHED	128

typedef struct tnode
{
    STATE	 *state;
    struct tnode *left, *right;

} TNODE;


PRIVATE TNODE	Heap[ MAX_UNFINISHED ]; /* Source of all TNODEs		  */
PRIVATE TNODE	*Next_allocate = Heap ; /* Ptr to next node to allocate   */

PRIVATE TNODE	 *Available  = NULL;	/* Free list of available nodes   */
					/* linked list of TNODES. p->left */
					/* is used as the link.		  */
PRIVATE TNODE	 *Unfinished = NULL;	/* Tree of unfinished states.	  */

PRIVATE ITEM	**State_items;		/* Used to pass info to state_cmp */
PRIVATE int	State_nitems;		/* 		"		  */
PRIVATE int	Sort_by_number = 0;	/*		"		  */

#define NEW	 0			/* Possible return values from 	  */
#define UNCLOSED 1			/* newstate().			  */
#define CLOSED	 2

ITEM	*Recycled_items = NULL;

#define MAX_TOK_PER_LINE  10
PRIVATE int Tokens_printed;	/* Controls number of lookaheads printed */
				/* on a single line of yyout.doc.	 */
#ifdef DEBUG
#ifdef __TURBOC__
#pragma warn -use
#endif
PRIVATE char *strprod       P((PRODUCTION *prod				     ));
#endif
void	add_action	  P(( int state, int input_sym, int do_this	     ));
void	add_goto	  P(( int state, int nonterminal, int go_here	     ));
int	add_lookahead	  P(( SET *dst, SET *src			     ));
void	addreductions	  P(( STATE *state, void *junk			     ));
void	add_unfinished	  P(( STATE *state				     ));
int	closure		  P(( STATE *kernel, ITEM **closure_items, \
					     int maxitems		     ));
int	do_close	  P(( ITEM *item,    ITEM **closure_items, \
					     int *nitems, int *maxitems	     ));
void	freeitem	  P(( ITEM *item				     ));
void	free_recycled_items P(( void 					     ));
STATE	*get_unfinished	  P(( void					     ));
ITEM	*in_closure_items P(( PRODUCTION *production, ITEM **closure_item, \
								int nitems   ));
int	item_cmp	  P(( ITEM **item1p, ITEM **item2p		     ));
int	kclosure	  P(( STATE *kernel, ITEM **closure_items,  \
					     int maxitems, int nclose	     ));
int	lr		  P(( STATE *cur_state				     ));
void	make_yy_lhs	  P(( PRODUCTION **prodtab			     ));
void	make_yy_reduce	  P(( PRODUCTION **prodtab			     ));
void	make_yy_slhs	  P(( PRODUCTION **prodtab			     ));
void	make_yy_srhs	  P(( PRODUCTION **prodtab			     ));
int	merge_lookaheads  P(( ITEM **dst_items, ITEM **src_items, int nitems ));
void	mkprod		  P(( SYMBOL *sym, PRODUCTION **prodtab		     ));
void	movedot		  P(( ITEM *item				     ));
int	move_eps	  P(( STATE *cur_state, ITEM **closure_items, \
						int nclose		     ));
MS  ( void *new )
UNIX( ACT  *new )  	  P(( void 					     ));
ITEM	*newitem	  P(( PRODUCTION *production			     ));
int	newstate	  P(( ITEM **items, int nitems, STATE **statep	     ));
ACT	*p_action	  P(( int state, int input_sym			     ));
void	pclosure	  P(( STATE *kernel, ITEM **closure_items, int nitems));
GOTO	*p_goto		  P(( int state, int nonterminal		     ));
void	print_reductions  P(( void					     ));
void	print_tab	  P(( ACT **table, char *row_name, char *col_name, \
							   int  private      ));
void	 pstate		  P(( STATE *state				     ));
void	 pstate_stdout	  P(( STATE *state				     ));
void	 reduce_one_item  P(( STATE *state, ITEM *item			     ));
void	 reductions	  P(( void					     ));
void	 sprint_tok	  P(( char **bp,     char *format, int arg	     ));
int	 state_cmp	  P(( STATE *new, STATE *tab_node		     ));
unsigned state_hash	  P(( STATE *sym				     ));
char	 *stritem	  P(( ITEM *item,    int lookaheads		     ));


void	lr_stats	  P(( FILE *fp ));			/* public */
int	lr_conflicts	  P(( FILE *fp ));
void	make_parse_tables P(( void     ));


  ANSI( PRIVATE void	*new( void ) )
  KnR ( PRIVATE ACT	*new() 	     )

{
    /* Return an area of memory that can be used as either an ACT or GOTO.
     * These objects cannot be freed.
     */

    static ACT  *eheap;	/* Assuming default initialization to NULL here */
    static ACT	*heap ; /* Ditto.					*/

    if( heap >= eheap )	   /* The > is cheap insurance, == is sufficient */
    {
	if( !(heap = (ACT *) malloc( sizeof(ACT) * CHUNK) ))
	    error( FATAL, "No memory for action or goto\n" );

	eheap = heap + CHUNK ;
    }
    ++Ntab_entries ;
    return heap++  ;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE ACT	*p_action( state, input_sym )
int state, input_sym;
{
    /* Return a pointer to the existing ACT structure representing the indicated
     * state and input symbol (or NULL if no such symbol exists).
     */

    ACT	*p;

    D( if( state > MAXSTATE )  						)
    D(    error(FATAL, "bad state argument to p_action (%d)\n", state);	)

    for( p = Actions[state]; p ; p = p->next )
	if( p->sym == input_sym )
	    return p;

    return NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE void	add_action( state, input_sym, do_this )
int state, input_sym, do_this;
{
    /* Add an element to the action part of the parse table. The cell is
     * indexed by the state number and input symbol, and holds do_this.
     */

    ACT	*p;

#ifdef INTERNAL
     if( state > MAXSTATE )
         error(FATAL, "bad state argument to add_action (%d)\n", state );

      if( (p = p_action(state, input_sym)) )
      {
  	error( FATAL,   "Tried to add duplicate action in state %d:\n"
  			"   (1) shift/reduce %d on %s <-existing\n"
  			"   (2) shift/reduce %d on %s <-new\n" ,
  			state, p->do_this, Terms[  p->sym ]->name,
  				  do_this, Terms[input_sym]->name );
     }
#endif

    if( Verbose > 1 )
	printf("Adding shift or reduce action from state %d:  %d on %s\n",
				      state, do_this, Terms[ input_sym ]->name);
    p		   = (ACT *) new();
    p->sym         = input_sym ;
    p->do_this     = do_this ;
    p->next        = Actions[state];
    Actions[state] = p;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE GOTO	*p_goto( state, nonterminal )
int state, nonterminal;
{
    /* Return a pointer to the existing GOTO structure representing the
     * indicated state and nonterminal (or NULL if no such symbol exists). The
     * value used for the nonterminal is the one in the symbol table; it is
     * adjusted down (so that the smallest nonterminal has the value 0)
     * before doing the table look up, however.
     */

    GOTO   *p;

    nonterminal = ADJ_VAL( nonterminal );

    D( if( nonterminal > NUMNONTERMS ) 			)
    D(    error(FATAL, "bad argument to p_goto\n");	)

    for( p = Gotos[ state ] ; p ; p = p->next )
	if( p->sym == nonterminal )
	    return p;

    return NULL;
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

PRIVATE void	add_goto( state, nonterminal, go_here )
int	state, nonterminal, go_here;
{
    /* Add an element to the goto part of the parse table, the cell is indexed
     * by current state number and nonterminal value, and holds go_here. Note
     * that the input nonterminal value is the one that appears in the symbol
     * table. It is adjusted downwards (so that the smallest nonterminal will
     * have the value 0) before being inserted into the table, however.
     */

    GOTO	*p;
    int		unadjusted;		/* Original value of nonterminal   */

    unadjusted  = nonterminal;
    nonterminal = ADJ_VAL( nonterminal );

#ifdef INTERNAL
      if( nonterminal > NUMNONTERMS )
          error(FATAL, "bad argument to add_goto\n");

      if( p = p_goto( state, unadjusted) )
          error(  FATAL,  "Tried to add duplicate goto on nonterminal %s\n"
      			"   (1) goto %3d from %3d <-existing\n"
      			"   (2) goto %3d from %3d <-new\n" ,
  			Terms[unadjusted]->name ,
  			p->go_here, p->state,
  			   go_here,    state );
#endif


    if( Verbose > 1 )
	    printf( "Adding goto from state %d to %d on %s\n",
				      state, go_here, Terms[unadjusted]->name );
    p            = (GOTO *) new();
    p->sym       = nonterminal;
    p->do_this   = go_here;
    p->next      = Gotos[state];
    Gotos[state] = p;
}

PRIVATE int	newstate( items, nitems, statep )
ITEM	**items;
int	nitems;
STATE	**statep;
{
    STATE  *state;
    STATE  *existing;
    int	   state_cmp() ;

    if( nitems > MAXKERNEL )
	error( FATAL, "Kernel of new state %d too large\n", Nstates );


    State_items  = items;	/* set up parameters for state_cmp */
    State_nitems = nitems;	/* and state_hash.		   */

    if( existing = (STATE *) findsym( States, NULL ) )
    {
	/* State exists; by not setting "state" to NULL, we'll recycle	*/
	/* the newly allocated state on the next call.			*/

	*statep = existing;
	if( Verbose > 1 )
	{
	    printf("Using existing state (%sclosed): ",
						existing->closed ? "" : "un" );
	    pstate_stdout( existing );
	}
	return existing->closed ? CLOSED : UNCLOSED ;
    }
    else
    {
	if( Nstates >= MAXSTATE )
	    error(FATAL, "Too many LALR(1) states\n");

	if( !(state = (STATE *) newsym(sizeof(STATE)) ))
	    error( FATAL, "Insufficient memory for states\n" );

	memcpy( state->kernel_items, items, nitems * sizeof(ITEM*) );
	state->nkitems  = nitems;
	state->neitems  = 0;
	state->closed   = 0;
	state->num 	= Nstates++ ;
	*statep 	= state;
	addsym( States, state );

	if( Verbose > 1 )
	{
	    printf("Forming new state:");
	    pstate_stdout( state );
	}

	return NEW;
    }
}

/*----------------------------------------------------------------------*/

PRIVATE void	add_unfinished( state )
STATE	*state;
{
    TNODE **parent, *root;
    int   cmp;

    parent = &Unfinished;
    root   = Unfinished;
    while( root )			/* look for the node in the tree */
    {
	if( (cmp = state->num - root->state->num) == 0 )
	    break;
	else
	{
	    parent = (cmp < 0) ? &root->left : &root->right ;
	    root   = (cmp < 0) ?  root->left :  root->right ;
	}
    }

    if( !root )				      /* Node isn't in tree.         */
    {
	if( Available )			      /* Allocate a new node and      */
	{				      /* put it into the tree.       */
	    *parent   = Available ;	      /* Use node from Available     */
	    Available = Available->left ;     /* list if possible, otherwise */
	}				      /* get the node from the Heap. */
	else
	{
	    if( Next_allocate >= &Heap[ MAX_UNFINISHED ] )
		error(FATAL, "Internal: No memory for unfinished state\n");
	    *parent = Next_allocate++;
	}

	(*parent)->state = state;		      /* initialize the node */
	(*parent)->left  = (*parent)->right = NULL;
    }

    D( printf("\nAdded state %d to unfinished tree:\n", state->num );	)
    D( prnt_unfin( Unfinished );					)
    D( printf("\n");							)
}

/*----------------------------------------------------------------------*/

PRIVATE STATE	*get_unfinished()
{
    /* Returns a pointer to the next unfinished state and deletes that
     * state from the unfinished tree. Returns NULL if the tree is empty.
     */

    TNODE	*root;
    TNODE	**parent;

    if( !Unfinished )
	return NULL;

    parent = &Unfinished;		/* find leftmost node */
    if( root = Unfinished )
    {
	while( root->left )
	{
	    parent = &root->left ;
	    root   = root->left ;
	}
    }

    *parent    = root->right ;	  	/* Unlink node from the tree	*/
    root->left = Available;	  	/* Put it into the free list	*/
    Available  = root;

    D(printf("\nReturning state %d from unfinished tree:\n",root->state->num);)
    D(prnt_unfin( Unfinished );	)
    D(printf("\n");		)

    return root->state ;
}

#ifdef DEBUG
  prnt_unfin( root )
  TNODE	*root;
  {
      if( !root )
  	return;

      prnt_unfin( root->left  );

      printf("Node %p, left=%p, right=%p, state=%p, state->num=%d\n",
  	    root, root->left, root->right, root->state, root->state->num );

      prnt_unfin( root->right );

  }
#endif


PRIVATE int	state_cmp( new, tab_node )
STATE	*new ;				/*  Pointer to new node (ignored */
					/*  if Sort_by_number is false). */
STATE	*tab_node;			/*  Pointer to existing node	 */
{
    /* Compare two states as described in the text. Return a number representing
     * the relative weight of the states, or 0 of the states are equivalent.
     */
    ITEM  **tab_item ; /* Array of items for existing state	*/
    ITEM  **item     ; /* Array of items for new state		*/
    int	  nitem      ; /* Size of "				*/
    int	  cmp 	     ;

    if( Sort_by_number )
	return( new->num - tab_node->num );

    if( cmp = State_nitems - tab_node->nkitems )	/* state with largest */
	return cmp;					/* number of items is */
							/* larger.	      */
    nitem    = State_nitems ;
    item     = State_items  ;
    tab_item = tab_node->kernel_items;

    for(; --nitem >= 0 ; ++tab_item, ++item )
    {
	if( cmp = (*item)->prod_num - (*tab_item)->prod_num )
	    return cmp;

	if( cmp = (*item)->dot_posn - (*tab_item)->dot_posn  )
	    return cmp;
    }
    return 0;					/* States are equivalent */
}

#ifdef __TURBOC__
#pragma argsused
#endif

PRIVATE unsigned state_hash( sym )
STATE	*sym;				/* ignored */
{
    /* Hash function for STATEs. Sum together production numbers and dot
     * positions of the kernel items.
     */
    ITEM     **items  ;	/* Array of items for new state		*/
    int	     nitems   ;	/* Size of "				*/
    unsigned total    ;

    items  = State_items  ;
    nitems = State_nitems ;
    total  = 0;

    for(;  --nitems >= 0 ; ++items )
	total += (*items)->prod_num  +  (*items)->dot_posn;
    return total;
}
PRIVATE ITEM	*newitem( production )
PRODUCTION	*production;
{
    ITEM       *item;

    if( Recycled_items )
    {
	item = Recycled_items ;
	Recycled_items = (ITEM *) Recycled_items->prod;
	CLEAR( item->lookaheads );
    }
    else
    {
	if( !(item = (ITEM *) malloc( sizeof(ITEM) )) )
	    error( FATAL, "Insufficient memory for all LR(1) items\n" );

	item->lookaheads = newset() ;
    }

#ifdef DEBUG
      if( !production )
  	  error( FATAL, "Illegal NULL production in newitem\n" );

      printf( "Making new item for %s (at 0x%p)\n", strprod(production), item );

      if( production->rhs[0] && production->rhs_len == 0 )
          error(FATAL, "Illegal epsilon production in newitem\n");
#endif

    ++Nitems;
    item->prod		=  production ;
    item->prod_num	=  production->num ;
    item->dot_posn	=  0;
    item->right_of_dot  =  production->rhs[0] ;
    return item;
}

PRIVATE void	freeitem( item )
ITEM	*item;
{
    --Nitems;
    item->prod = (PRODUCTION *) Recycled_items ;
    Recycled_items = item;
}

PRIVATE void	free_recycled_items()
{
    /* empty the recycling heap, freeing all memory used by items there */

    ITEM *p;

    while( p = Recycled_items )
    {
	Recycled_items = (ITEM *) Recycled_items->prod ;
	free(p);
    }
}

PRIVATE void movedot( item )
ITEM	*item;
{
    /* Moves the dot one position to the right and updates the right_of_dot
     * symbol.
     */

    D( if( item->right_of_dot == NULL )					  )
    D( 	  error(FATAL,"Illegal movedot() call on epsilon production\n");  )

    item->right_of_dot = ( item->prod->rhs )[ ++item->dot_posn ] ;
}

PRIVATE int   item_cmp( item1p,  item2p )
ITEM  **item1p, **item2p ;
{
    /* Return the relative weight of two items, 0 if they're equivalent.  */

    int rval;
    ITEM *item1 = *item1p;
    ITEM *item2 = *item2p;

    if( !(rval = RIGHT_OF_DOT(item1) - RIGHT_OF_DOT(item2)) )
	if( !(rval = item1->prod_num - item2->prod_num ) )
	    return item1->dot_posn - item2->dot_posn ;

    return rval;
}

#ifdef NEVER
    D( printf("Comparing %s\n", stritem( *item1p, 0) );			      )
    D( printf("      and %s\n", stritem( *item2p, 0) );			      )
    D( printf("Symbol to right of dot in item1 = %d\n", RIGHT_OF_DOT(item1)); )
    D( printf("Symbol to right of dot in item2 = %d\n", RIGHT_OF_DOT(item2)); )
    D( printf("item1->prod_num = %d\n", item1->prod_num );		      )
    D( printf("item2->prod_num = %d\n", item2->prod_num );		      )
    D( printf("item1->dot_posn  = %d\n", item1->dot_posn  );		      )
    D( printf("item2->dot_posn  = %d\n", item2->dot_posn  );		      )
    D( printf("Returning %d\n\n",	 rval 		  );		      )
#endif
PUBLIC void	make_parse_tables()
{
    /* Prints an LALR(1) transition matrix for the grammar currently
     * represented in the symbol table.
     */

    ITEM	*item;
    STATE	*state;
    PRODUCTION	*start_prod;
    void	mkstates();
    int		state_cmp();
    FILE	*fp, *old_output;

    /* Make data structures used to produce the table, and create an initial
     * LR(1) item containing the start production and the end-of-input marker
     * as a lookahead symbol.
     */

    States = maketab( 257, state_hash, state_cmp );

    if( !Goal_symbol )
	error(FATAL, "No goal symbol.\n" );

    start_prod = Goal_symbol->productions ;

    if( start_prod->next )
	error(FATAL, "Start symbol must have only one right-hand side.\n" );

    item = newitem( start_prod );	    /* Make item for start production */
    ADD( item->lookaheads, _EOI_ );	    /* FOLLOW(S) = {$} 		      */

    newstate( &item, 1, &state );

    if( lr( state ) )			 /* Add shifts and gotos to the table */
    {
	if( Verbose )
	    printf("Adding reductions:\n");

	reductions();					/* add the reductions */

	if( Verbose )
	    printf("Creating tables:\n" );

	if( !Make_yyoutab )			     /* Tables go in yyout.c */
	{
	    print_tab( Actions, "Yya", "Yy_action", 1);
	    print_tab( Gotos  , "Yyg", "Yy_goto"  , 1);
	}
	else
	{
	    if( !(fp = fopen(TAB_FILE, "w")) )
	    {
		error( NONFATAL, "Can't open %s, ignoring -T\n", TAB_FILE);
		print_tab( Actions, "Yya", "Yy_action", 1);
		print_tab( Gotos  , "Yyg", "Yy_goto"  , 1);
	    }
	    else
	    {
		output ( "extern YY_TTYPE *Yy_action[]; /* in yyoutab.c */\n" );
		output ( "extern YY_TTYPE *Yy_goto[];   /* in yyoutab.c */\n" );

		old_output = Output;
		Output     = fp;

		fprintf(fp, "#include <stdio.h>\n" );
		fprintf(fp, "typedef short YY_TTYPE;\n" );
		fprintf(fp, "#define YYPRIVATE %s\n",
					Public ? "/* empty */" : "static" );

		print_tab( Actions, "Yya", "Yy_action", 0 );
		print_tab( Gotos  , "Yyg", "Yy_goto"  , 0);

		fclose( fp );
		Output = old_output;
	    }
	}
	print_reductions();
    }
}



PRIVATE int lr( cur_state )
STATE  *cur_state;
{
    /* Make LALR(1) state machine. The shifts and gotos are done here, the
     * reductions are done elsewhere. Return the number of states.
     */

    ITEM   **p;
    ITEM   **first_item;
    ITEM   *closure_items[ MAXCLOSE ];
    STATE  *next;		/* Next state.				    */
    int	   isnew;		/* Next state is a new state.		    */
    int	   nclose;		/* Number of items in closure_items.	    */
    int	   nitems;		/* # items with same symbol to right of dot.*/
    int	   val;			/* Value of symbol to right of dot.	    */
    SYMBOL *sym;		/* Actual symbol to right of dot.	    */
    int	   nlr  = 0;		/* Nstates + nlr == number of LR(1) states. */

    add_unfinished( cur_state );

    while( cur_state = get_unfinished() )
    {
	if( Verbose > 1 )
		printf( "Next pass...working on state %d\n", cur_state->num );

	/* closure()  adds normal closure items to closure_items array.
	 * kclose()   adds to that set all items in the kernel that have
	 *            outgoing transitions (ie. whose dots aren't at the far
	 *	      right).
	 * assort()   sorts the closure items by the symbol to the right
	 *	      of the dot. Epsilon transitions will sort to the head of
	 *	      the list, followed by transitions on nonterminals,
	 *	      followed by transitions on terminals.
	 * move_eps() moves the epsilon transitions into the closure kernel set.
	 *            It returns the number of items that it moved.
	 */

	nclose = closure  (cur_state, closure_items, MAXCLOSE 		);
	nclose = kclosure (cur_state, closure_items, MAXCLOSE, nclose	);

	if( nclose == 0 )
	{
	    if( Verbose > 1 )
		printf("There were NO closure items added\n");
	}
	else
	{
	    assort( (void**)closure_items, nclose, sizeof(ITEM*),
					    (int (*)(void*,void*)) item_cmp );
	    nitems  = move_eps( cur_state, closure_items, nclose );
	    p       = closure_items + nitems;
	    nclose -= nitems ;

	    if( Verbose > 1 )
		pclosure( cur_state, p, nclose );
	}

	/* All of the remaining items have at least one symbol to the	*/
	/* right of the dot.						*/

	while( nclose > 0 )  	/* fails immediatly if no closure items */
	{
	    first_item = p ;
	    sym	       = (*first_item)->right_of_dot ;
	    val	       = sym->val ;

	    /* Collect all items with the same symbol to the right of the dot.
	     * On exiting the loop, nitems will hold the number of these items
	     * and p will point at the first nonmatching item. Finally nclose is
	     * decremented by nitems. items = 0 ;
	     */

	    nitems = 0 ;
	    do
	    {
		movedot( *p++ );
		++nitems;

	    } while( --nclose > 0  &&  RIGHT_OF_DOT(*p) == val );

	    /* (1) newstate() gets the next state. It returns NEW if the state
	     *	   didn't exist previously, CLOSED if LR(0) closure has been
	     *     performed on the state, UNCLOSED otherwise.
	     * (2) add a transition from the current state to the next state.
	     * (3) If it's a brand-new state, add it to the unfinished list.
	     * (4) otherwise merge the lookaheads created by the current closure
	     *     operation with the ones already in the state.
	     * (5) If the merge operation added lookaheads to the existing set,
	     *     add it to the unfinished list.
	     */

	    isnew = newstate( first_item, nitems, &next );		/* 1 */

	    if( !cur_state->closed )
	    {
		if( ISTERM( sym ) )					/* 2 */
		    add_action ( cur_state->num, val, next->num );
		else
		    add_goto   ( cur_state->num, val, next->num );
	    }

	    if( isnew == NEW )
		add_unfinished( next );					/* 3 */
	    else
	    {								/* 4 */
		if( merge_lookaheads( next->kernel_items, first_item, nitems))
		{
		    add_unfinished( next );				/* 5 */
		    ++nlr;
		}
		while( --nitems >= 0 )
		    freeitem( *first_item++ );
	    }

	    fprintf( stderr, "\rLR:%-3d LALR:%-3d", Nstates + nlr, Nstates );
	}
	cur_state->closed = 1;
    }

    free_recycled_items();

    if( Verbose )
	fprintf(stderr, " states, %d items, %d shift and goto transitions\n",
					    Nitems, Ntab_entries );
    return Nstates;
}

/*----------------------------------------------------------------------*/

PRIVATE int 	merge_lookaheads( dst_items, src_items, nitems )
ITEM	**src_items;
ITEM	**dst_items;
int	nitems;
{
    /* This routine is called if newstate has determined that a state having the
     * specified items already exists. If this is the case, the item list in the
     * STATE and the current item list will be identical in all respects except
     * lookaheads. This routine merges the lookaheads of the input items
     * (src_items) to the items already in the state (dst_items). 0 is returned
     * if nothing was done (all lookaheads in the new state are already in the
     * existing state), 1 otherwise. It's an internal error if the items don't
     * match.
     */

    int did_something = 0;

    while( --nitems >= 0 )
    {
	if( 	(*dst_items)->prod     != (*src_items)->prod
	    || 	(*dst_items)->dot_posn != (*src_items)->dot_posn )
	{
	    error(FATAL, "INTERNAL [merge_lookaheads], item mismatch\n" );
	}

	if( !subset( (*dst_items)->lookaheads, (*src_items)->lookaheads ) )
	{
	    ++did_something;
	    UNION( (*dst_items)->lookaheads, (*src_items)->lookaheads );
	}

	++dst_items;
	++src_items;
    }

    return did_something;
}

/*----------------------------------------------------------------------*/

PRIVATE int	move_eps( cur_state, closure_items, nclose )
STATE	*cur_state;
ITEM	**closure_items;
int	nclose;
{
    /* Move the epsilon items from the closure_items set to the kernel of the
     * current state. If epsilon items already exist in the current state,
     * just merge the lookaheads. Note that, because the closure items were
     * sorted to partition them, the epsilon productions in the closure_items
     * set will be in the same order as those already in the kernel. Return
     * the number of items that were moved.
     */

    ITEM **eps_items, **p ;
    int  nitems, moved ;

    eps_items = cur_state->epsilon_items;
    nitems    = cur_state->neitems ;
    moved     = 0;

    D( {								 )
    D( 	    extern int _psp;						 )
    D( 	    void huge* p;						 )
    D(	    void huge *q = (*closure_items)->prod;		 	 )
    D( 	    SEG(p) = _psp;						 )
    D( 	    OFF(p) = 0 ;						 )

    D( 	    if( PHYS(closure_items) < PHYS(p) || PHYS(q) < PHYS(p))	 )
    D(	 	error(FATAL, "Bad pointer in move_eps\n" );		 )
    D( }								 )

    for( p = closure_items; (*p)->prod->rhs_len == 0 && --nclose >= 0; )
    {
	if( ++moved > MAXEPSILON )
	    error(FATAL, "Too many epsilon productions in state %d\n",
						 cur_state->num );
	if( nitems )
	    UNION( (*eps_items++)->lookaheads, (*p++)->lookaheads );
	else
	    *eps_items++ = *p++ ;

        D( if( !p || !(*p)->prod )				)
      	D(     error(FATAL, "Bad pointer in move_eps\n" );	)

    }

    if( moved )
	cur_state->neitems = moved ;

    return moved ;
}

/*----------------------------------------------------------------------*/

PRIVATE int   kclosure( kernel, closure_items, maxitems, nclose )
STATE *kernel;			/* Kernel state to close.		   */
ITEM  **closure_items;		/* Array into which closure items are put. */
int   maxitems;			/* Size of the closure_items[] array.	   */
int   nclose;			/* # of items already in set.		   */
{
    /* Adds to the closure set those items from the kernel that will shift to
     * new states (ie. the items with dots somewhere other than the far right).
     */

    int 	nitems;
    ITEM	*item, **itemp, *citem ;

    closure_items += nclose;			/* Correct for existing items */
    maxitems      -= nclose;

    itemp  = kernel->kernel_items ;
    nitems = kernel->nkitems ;

    while( --nitems >= 0 )
    {
	item = *itemp++;

	if( item->right_of_dot )
	{
	    citem 		= newitem( item->prod );
    	    citem->prod		= item->prod ;
    	    citem->dot_posn     = item->dot_posn ;
            citem->right_of_dot = item->right_of_dot ;
    	    citem->lookaheads	= dupset( item->lookaheads );

	    if( --maxitems < 0 )
		error( FATAL, "Too many closure items in state %d\n",
								kernel->num );
	    *closure_items++ = citem;
	    ++nclose;
	}
    }
    return nclose;
}

/*----------------------------------------------------------------------*/

PRIVATE int closure( kernel, closure_items, maxitems )
STATE *kernel;			/* Kernel state to close.		  */
ITEM  *closure_items[];		/* Array into which closure items are put */
int   maxitems;			/* Size of the closure_items[] array.	  */
{
    /* Do LR(1) closure on the kernel items array in the input STATE. When
     * finished, closure_items[] will hold the new items. The logic is:
     *
     * (1) for( each kernel item )
     *         do LR(1) closure on that item.
     * (2) while( items were added in the previous step or are added below )
     *         do LR(1) closure on the items that were added.
     */

    int  i ;
    int  nclose 	= 0 ;		/* Number of closure items */
    int  did_something  = 0 ;
    ITEM **p 		= kernel->kernel_items ;

    for( i = kernel->nkitems; --i >= 0 ;)			       /* (1) */
    {
	did_something |= do_close( *p++, closure_items, &nclose, &maxitems );
    }

    while( did_something )					       /* (2) */
    {
	did_something = 0;
	p = closure_items;
	for( i = nclose ; --i >= 0 ; )
	    did_something |= do_close( *p++, closure_items, &nclose, &maxitems);
    }

    return nclose;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

PRIVATE int   do_close( item, closure_items, nitems, maxitems )
ITEM  *item;
ITEM  *closure_items[];	/* (output) Array of items added by closure process */
int   *nitems;		/* (input)  # of items currently in closure_items[] */
			/* (output) # of items in closure_items after       */
			/*  					 processing */
int   *maxitems;	/* (input)  max # of items that can be added        */
			/* (output) input adjusted for newly added items    */
{
    /* Workhorse function used by closure(). Performs LR(1) closure on the
     * input item ([A->b.Cd, e] add [C->x, FIRST(de)]). The new items are added
     * to the closure_items[] array and *nitems and *maxitems are modified to
     * reflect the number of items in the closure set. Return 1 if do_close()
     * did anything, 0 if no items were added (as will be the case if the dot
     * is at the far right of the production or the symbol to the right of the
     * dot is a terminal).
     */

    int 	did_something = 0;
    int		rhs_is_nullable;
    PRODUCTION	*prod;
    ITEM	*close_item;
    SET		*closure_set;
    SYMBOL	**symp;

    if( !item->right_of_dot )
	return 0;

    if( !ISNONTERM( item->right_of_dot ) )
	return 0;

    closure_set = newset();

    /* The symbol to the right of the dot is a nonterminal. Do the following:
     *
     *(1)  for( every production attached to that nonterminal )
     *(2)	if( the current production is not already in the set of
     *								closure items)
     *(3)            add it;
     *(4)       if( the d in [A->b.Cd, e] doesn't exist )
     *(5)            add e to the lookaheads in the closure production.
     *	        else
     *(6)	    The d in [A->b.Cd, e] does exist, compute FIRST(de) and add
     *		    it to the lookaheads for the current item if necessary.
     */
								       /* (1) */

    for( prod = item->right_of_dot->productions; prod ; prod = prod->next )
    {
								       /* (2) */
	if( !(close_item = in_closure_items(prod, closure_items, *nitems)))
        {
	    if( --(*maxitems) <= 0 )
		error(FATAL, "LR(1) Closure set too large\n" );
								       /* (3) */
	    closure_items[ (*nitems)++ ] = close_item = newitem( prod );
	    ++did_something;
	}

	if( !*(symp = & ( item->prod->rhs [ item->dot_posn + 1 ])) )   /* (4) */
	{
	    did_something |= add_lookahead( close_item->lookaheads,    /* (5) */
							     item->lookaheads );
	}
	else
	{
	    truncate( closure_set );				       /* (6) */

	    rhs_is_nullable = first_rhs( closure_set, symp,
				    item->prod->rhs_len - item->dot_posn - 1 );

	    REMOVE( closure_set, EPSILON );

	    if( rhs_is_nullable )
		UNION( closure_set, item->lookaheads );

	    did_something |= add_lookahead(close_item->lookaheads, closure_set);
	}
    }

    delset( closure_set );
    return did_something;
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

PRIVATE ITEM	  *in_closure_items(production, closure_item, nitems)
PRODUCTION *production;
ITEM	   **closure_item;
int	   nitems;
{
    /* If the indicated production is in the closure_items already, return a
     * pointer to the existing item, otherwise return NULL.
     */

    for(; --nitems >= 0 ; ++closure_item )
	if( (*closure_item)->prod == production )
	    return *closure_item;

    return NULL;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
PRIVATE int	add_lookahead( dst, src )
SET	*dst, *src;
{
    /* Merge the lookaheads in the src and dst sets. If the original src
     * set was empty, or if it was already a subset of the destination set,
     * return 0, otherwise return 1.
     */

    if( !IS_EMPTY( src ) && !subset( dst, src ) )
    {
	UNION( dst, src );
	return 1;
    }

    return 0;
}
PRIVATE void	reductions()
{
    /* Do the reductions. If there's memory, sort the table by state number */
    /* first so that yyout.doc will look nice.				    */

    void  addreductions();					/* below */

    Sort_by_number = 1;
    if( !ptab( States, (ptab_t)addreductions, NULL, 1 ) )
	ptab( States, (ptab_t)addreductions, NULL, 0 );
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef __TURBOC__
#pragma argsused
#endif

PRIVATE void addreductions( state, junk )
STATE	*state;
void	*junk;
{
    /* This routine is called for each state. It adds the reductions using the
     * disambiguating rules described in the text, and then prints the state to
     * yyout.doc if Verbose is true. I don't like the idea of doing two things
     * at once, but it makes for nicer output because the error messages will
     * be next to the state that caused the error.
     */

    int	  	i;
    ITEM  	**item_p;

     D( printf("----------------------------------\n");	)
     D( pstate_stdout( state );  			)
     D( printf("- - - - - - - - - - - - - - - - -\n");	)

    for( i = state->nkitems, item_p = state->kernel_items;  --i>=0; ++item_p )
	reduce_one_item( state, *item_p );

    for( i = state->neitems, item_p = state->epsilon_items; --i>=0; ++item_p )
	reduce_one_item( state, *item_p );

    if( Verbose )
    {
	pstate( state );

	if( state->num % 10 == 0 )
	    fprintf( stderr, "%d\r", state->num );
    }
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

PRIVATE void	reduce_one_item( state, item )
ITEM	*item;				/* Reduce on this item	*/
STATE	*state;				/* from this state	*/
{
    int		token;	  		/* Current lookahead	        */
    int		pprec;	  		/* Precedence of production	*/
    int		tprec;	  		/* Precedence of token 		*/
    int		assoc;	  		/* Associativity of token	*/
    int		reduce_by;
    int		resolved;		/* True if conflict can be resolved */
    ACT		*ap;

    if( item->right_of_dot )		/* No reduction required */
	    return;

    pprec = item->prod->prec ;		/* precedence of entire production */

    D( printf( "ITEM:  %s\n", stritem(item, 2) ); )

    for( next_member(NULL); (token = next_member(item->lookaheads)) >= 0 ;)
    {
	tprec = Precedence[token].level ;	/* precedence of lookahead */
	assoc = Precedence[token].assoc ;	/* symbol.		   */

  	D( printf( "TOKEN: %s (prec=%d, assoc=%c)\n", Terms[token]->name, \
  							     tprec, assoc ); )

	if( !(ap = p_action( state->num, token )) )	/* No conflicts */
	{
	    add_action( state->num, token, -(item->prod_num) );

  	    D( printf( "Action[%d][%s]=%d\n", state->num, \
  				Terms[token]->name, -(item->prod_num) ); )
	}
	else if( ap->do_this <= 0 )
	{
	    /* Resolve a reduce/reduce conflict in favor of the production */
	    /* with the smaller number. Print a warning.		   */

	    ++Reduce_reduce;

	    reduce_by   = min( -(ap->do_this), item->prod_num );
	    ap->do_this = -reduce_by ;

	    error( WARNING, "State %2d: reduce/reduce conflict ", state->num );
	    error( NOHDR,   "%d/%d on %s (choose %d).\n",
				    -(ap->do_this), item->prod_num ,
				    token ? Terms[token]->name: "<_EOI_>",
				    reduce_by				     );
	}
	else				 /* Shift/reduce conflict. */
	{
	    if( resolved = (pprec && tprec) )
		if( tprec < pprec || (pprec == tprec && assoc != 'r')  )
		    ap->do_this = -( item->prod_num );

	    if( Verbose > 1 || !resolved )
	    {
		++Shift_reduce;
		error( WARNING, "State %2d: shift/reduce conflict", state->num);
		error( NOHDR,	" %s/%d (choose %s) %s\n",
				Terms[token]->name,
				item->prod_num,
				ap->do_this < 0 ? "reduce"     : "shift",
				resolved        ? "(resolved)" : ""
									    );
	    }
	}
    }
}
PUBLIC void lr_stats( fp )
FILE	*fp;
{
    /*  Print out various statistics about the table-making process */

    fprintf(fp, "%4d      LALR(1) states\n",	 	   Nstates );
    fprintf(fp, "%4d      items\n",		 	   Nitems  );
    fprintf(fp, "%4d      nonerror transitions in tables\n", Ntab_entries  );
    fprintf(fp, "%4ld/%-4d unfinished items\n", (long)(Next_allocate - Heap),
							    MAX_UNFINISHED);
    fprintf(fp, "%4d      bytes required for LALR(1) transition matrix\n",
			      (2 * sizeof(char*) * Nstates) /* index arrays */
			      + Nstates			    /* count fields */
			      + (Npairs * sizeof(short))    /* pairs        */
	   );
    fprintf(fp, "\n");
}
/*----------------------------------------------------------------------*/
PUBLIC int lr_conflicts( fp )
FILE	*fp;
{
    /* Print out statistics for the inadequate states and return the number of
     * conflicts.
     */

    fprintf(fp, "%4d      shift/reduce  conflicts\n",  Shift_reduce  );
    fprintf(fp, "%4d      reduce/reduce conflicts\n",  Reduce_reduce );
    return Shift_reduce + Reduce_reduce ;
}

#if ( (0 ANSI(+1)) == 0 )	/* if not an ANSI compiler */
#define sprintf _sprintf
#endif
#ifdef __TURBOC__
#pragma argsused
#endif

PRIVATE void sprint_tok( bp, format, arg )
char	**bp;
char	*format;		/* not used here, but supplied by pset() */
int	arg;
{
    /* Print one nonterminal symbol to a buffer maintained by the
     * calling routine and update the calling routine's pointer.
     */

    if     ( arg == -1     ) *bp += sprintf( *bp, "null "		  );
    else if( arg == -2     ) *bp += sprintf( *bp, "empty "		  );
    else if( arg == _EOI_  ) *bp += sprintf( *bp, "$ "			  );
    else if( arg == EPSILON) *bp += sprintf( *bp, ""			  );
    else	             *bp += sprintf( *bp, "%s ", Terms[arg]->name );

    if( ++Tokens_printed >= MAX_TOK_PER_LINE )
    {
	*bp += sprintf(*bp, "\n\t\t");
	Tokens_printed = 0;
    }
}
/*----------------------------------------------------------------------*/
PRIVATE  char 	*stritem( item, lookaheads )
ITEM	*item;
int	lookaheads;
{
    /* Return a pointer to a string that holds a representation of an item.
     * The lookaheads are printed too if "lookaheads" is true or Verbose
     * is > 1 (-V was specified on the command line).
     */

    static char buf[ MAXOBUF * 4 ];
    char	*bp;
    int	   	i;

    bp  = buf;
    bp += sprintf( bp, "%s->", item->prod->lhs->name );

    if( item->prod->rhs_len <= 0 )
	bp += sprintf( bp, "<epsilon>. " );
    else
    {
	for( i = 0; i < item->prod->rhs_len ; ++i )
	{
	    if( i == item->dot_posn )
		*bp++  = '.' ;

	    bp += sprintf(bp, " %s", item->prod->rhs[i]->name );
	}
	if( i == item->dot_posn )
	    *bp++  = '.' ;
    }

    if( lookaheads || Verbose >1 )
    {
	bp += sprintf( bp, " (production %d, precedence %d)\n\t\t[",
					item->prod->num, item->prod->prec );
	Tokens_printed = 0;
	pset( item->lookaheads, (pset_t)sprint_tok, &bp );
	*bp++ = ']'  ;
    }

    if(  bp >= &buf[sizeof(buf)]   )
	error(FATAL, "Internal [stritem], buffer overflow\n" );

    *bp = '\0' ;
    return buf;
}
/*----------------------------------------------------------------------*/
PRIVATE void	pstate( state )
STATE	*state;
{
    /* Print one row of the parse table in human-readable form yyout.doc
     * (stderr if -V is specified).
     */

    int   	i;
    ITEM  	**item;
    ACT		*p;

    document( "State %d:\n", state->num );

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
    /* Print the kernel and epsilon items for the current state.  */

    for( i=state->nkitems, item=state->kernel_items  ; --i >= 0 ; ++item )
	document("    %s\n", stritem(*item, (*item)->right_of_dot==0 ));

    for( i=state->neitems, item=state->epsilon_items ; --i >= 0 ; ++item )
	document( "    %s\n", stritem(*item, 1) );

    document( "\n" );

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
    /* Print out the next-state transitions, first the actions,   */
    /* then the gotos.						  */

    for( i = 0; i < (MINTERM+USED_TERMS); ++i )
    {
	if( p = p_action( state->num, i ) )
	{
	    if( p->do_this == 0 )
	    {
		if( p->sym == _EOI_ )
		    document( "    Accept on end of input\n" );
		else
		    error( FATAL, "INTERNAL: state %d, Illegal accept",
								    state->num);
	    }
	    else if( p->do_this < 0 )
		document( "    Reduce by %d on %s\n", -(p->do_this),
							Terms[p->sym]->name );
	    else
		document( "    Shift to %d on %s\n", p->do_this,
							Terms[p->sym]->name );
	}
    }

    for( i = MINNONTERM; i < MINNONTERM + USED_NONTERMS; i++ )
	if( p = p_goto(state->num, i) )
	    document( "    Goto %d on %s\n", p->do_this,
							Terms[i]->name );
    document("\n");
}
/*----------------------------------------------------------------------*/
PRIVATE void	pstate_stdout( state )
STATE	*state;
{
    document_to( stdout );
    pstate( state );
    document_to( NULL );
}
/*----------------------------------------------------------------------*/
PRIVATE void pclosure( kernel, closure_items, nitems )
STATE	*kernel;
ITEM	**closure_items;
int	nitems;
{
    printf( "\n%d items in Closure of ", nitems );
    pstate_stdout( kernel );

    if( nitems > 0 )
    {
	printf( "    -----closure items:----\n" );
	while( --nitems >= 0 )
	    printf( "    %s\n", stritem( *closure_items++, 0) );
    }
}

PRIVATE void	make_yy_lhs( prodtab )
PRODUCTION	**prodtab;
{
    static char *text[] =
    {
	"The Yy_lhs array is used for reductions. It is indexed by production",
	"number and holds the associated left-hand side adjusted so that the",
	"number can be used as an index into Yy_goto.",
	NULL
    };
    PRODUCTION *prod;
    int		i;

    comment ( Output, text );
    output  ( "YYPRIVATE int Yy_lhs[%d] =\n{\n", Num_productions );

    for( i = 0; i < Num_productions; ++i )
    {
	prod = *prodtab++;
	output("\t/* %3d */\t%d", prod->num, ADJ_VAL( prod->lhs->val ) );

	if( i != Num_productions-1 )
	    output(",");

	if( i % 3 == 2 || i == Num_productions-1 )	/* use three columns */
	    output( "\n" );
    }
    output("};\n");
}

/*----------------------------------------------------------------------*/

PRIVATE void	make_yy_reduce( prodtab )
PRODUCTION	**prodtab;
{
    static char *text[] =
    {
	"The Yy_reduce[] array is indexed by production number and holds",
	"the number of symbols on the right-hand side of the production",
	NULL
    };
    PRODUCTION *prod;
    int		i;

    comment ( Output, text );
    output  ( "YYPRIVATE int Yy_reduce[%d] =\n{\n", Num_productions );

    for( i = 0; i < Num_productions; ++i )
    {
	prod = *prodtab++;
	output( "\t/* %3d */\t%d", prod->num, prod->rhs_len );

	if( i != Num_productions-1 )
	    output(",");

	if( i % 3 == 2 || i == Num_productions-1 )	/* use three columns */
	    output( "\n" );
    }
    output("};\n");
}

/*----------------------------------------------------------------------*/

PRIVATE void make_yy_slhs( prodtab )
PRODUCTION	**prodtab;
{
    static char *text[] =
    {
	"Yy_slhs[] is a debugging version of Yy_lhs[]. It is indexed by",
	"production number and evaluates to a string representing the",
	"left-hand side of the production.",
	NULL
    };

    PRODUCTION *prod;
    int		i;

    comment ( Output, text );
    output  ( "YYPRIVATE char *Yy_slhs[%d] =\n{\n", Num_productions );

    for( i = Num_productions; --i >= 0 ; )
    {
	prod = *prodtab++;
	output("\t/* %3d */\t\"%s\"", prod->num, prod->lhs->name );
	output( i != 0 ? ",\n" : "\n" );
    }
    output("};\n");
}

PRIVATE void make_yy_srhs( prodtab )
PRODUCTION	**prodtab;
{
    static char *text[] =
    {
	"Yy_srhs[] is also used for debugging. It is indexed by production",
	"number and evaluates to a string representing the right-hand side of",
	"the production.",
	NULL
    };

    PRODUCTION *prod;
    int		i, j;

    comment ( Output, text );
    output  ( "YYPRIVATE char *Yy_srhs[%d] =\n{\n", Num_productions );

    for( i = Num_productions; --i >= 0 ; )
    {
	prod = *prodtab++;
	output("\t/* %3d */\t\"",  prod->num );

	for( j = 0; j < prod->rhs_len ; ++j )
	{
	    output( "%s", prod->rhs[j]->name );
	    if( j != prod->rhs_len - 1 )
		outc( ' ' );
	}
	output( i != 0 ? "\",\n" : "\"\n" );
    }
    output("};\n");
}

/*----------------------------------------------------------------------
 * The following routines generate compressed parse tables. There's currently
 * no way to do uncompressed tables. The default transition is the error
 * transition.
 */

PRIVATE void	print_reductions()
{
    /* Output the various tables needed to do reductions */

    PRODUCTION	**prodtab;

    if(!(prodtab= (PRODUCTION**) malloc(sizeof(PRODUCTION*) * Num_productions)))
	error(FATAL,"Not enough memory to output LALR(1) reduction tables\n");
    else
	ptab( Symtab, (ptab_t)mkprod, prodtab, 0 );

    make_yy_lhs    ( prodtab );
    make_yy_reduce ( prodtab );

    output("#ifdef YYDEBUG\n");

    make_yy_slhs   ( prodtab );
    make_yy_srhs   ( prodtab );

    output("#endif\n");
    free( prodtab );
}

/*----------------------------------------------------------------------*/

PRIVATE void	mkprod( sym, prodtab )
SYMBOL		*sym;
PRODUCTION	**prodtab;
{
    PRODUCTION	*p;

    if( ISNONTERM(sym) )
	for( p = sym->productions ; p ; p = p->next )
  	{
#ifdef INTERNAL
  		if( p->num >= Num_productions )
  		    error( FATAL,"Internal [mkprod], bad prod. num.\n" );
#endif

	    prodtab[ p->num ] = p ;
  	}
}

/*----------------------------------------------------------------------*/

PRIVATE void	print_tab( table, row_name, col_name, make_private )
ACT	**table;
char	*row_name;	/* Name to use for the row arrays 	  	   */
char	*col_name;	/* Name to use for the row-pointers array 	   */
int	make_private;	/* Make index table private (rows always private)  */
{
    /* Output the action or goto table. */

    int		i, j;
    ACT		*ele, **elep; /* table element and pointer to same	     */
    ACT		*e, **p;
    int		count;        /* # of transitions from this state, always >0 */
    int 	column;
    SET		*redundant = newset();            /* Mark redundant rows */

    static char	*act_text[] =
    {
	"The Yy_action table is action part of the LALR(1) transition",
	"matrix. It's compressed and can be accessed using the yy_next()",
	"subroutine, declared below.",
	"",
	"             Yya000[]={   3,   5,3   ,   2,2   ,   1,1   };",
	"  state number---+        |    | |",
	"  number of pairs in list-+    | |",
	"  input symbol (terminal)------+ |",
	"  action-------------------------+",
	"",
	"  action = yy_next( Yy_action, cur_state, lookahead_symbol );",
	"",
	"	action <  0   -- Reduce by production n,  n == -action.",
	"	action == 0   -- Accept. (ie. Reduce by production 0.)",
	"	action >  0   -- Shift to state n,  n == action.",
	"	action == YYF -- error.",
	NULL
    };
    static char	*goto_text[] =
    {
	"The Yy_goto table is goto part of the LALR(1) transition matrix",
	"",
	" nonterminal = Yy_lhs[ production number by which we just reduced ]",
	"",
	"              Yyg000[]={   3,   5,3   ,   2,2   ,   1,1   };",
	"  uncovered state-+        |    | |",
	"  number of pairs in list--+    | |",
	"  nonterminal-------------------+ |",
	"  goto this state-----------------+",
	"",
	"It's compressed and can be accessed using the yy_next() subroutine,",
	"declared below, like this:",
	"",
	"  goto_state = yy_next( Yy_goto, cur_state, nonterminal );",
	NULL
    };

    comment( Output, table == Actions ? act_text : goto_text );

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Modify the matrix so that, if a duplicate rows exists, only one
     * copy of it is kept around. The extra rows are marked as such by setting
     * a bit in the "redundant" set. (The memory used for the chains is just
     * discarded.) The redundant table element is made to point at the row
     * that it duplicates.
     */

    for( elep = table, i = 0; i < Nstates ; ++elep, ++i )
    {
	if( MEMBER( redundant,i ) )
	    continue;

	for( p=elep+1, j=i ; ++j < Nstates ; ++p )
	{
	    if( MEMBER( redundant, j) )
		continue;

	    ele = *elep;	/* pointer to template chain		*/
	    e   = *p;		/* chain to compare against template	*/

	    if( !e || !ele )	/* either or both strings have no elements */
		continue;

	    for( ; ele && e ; ele=ele->next, e=e->next )
		if( (ele->do_this != e->do_this) || (ele->sym != e->sym) )
		    break;

	    if( !e && !ele )
	    {
		/* Then the chains are the same. Mark the chain being compared
		 * as redundant, and modify table[j] to hold a pointer to the
		 * template pointer.
		 */

		ADD( redundant, j );
		table[j] = (ACT *) elep;
	    }
	}
    }

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Output the row arrays
     */

    for( elep = table, i = 0 ; i < Nstates ; ++elep, ++i )
    {
	if( !*elep || MEMBER(redundant, i) )
	    continue;
			   /* Count the number of transitions from this state */
	count = 0;
	for( ele = *elep ; ele ; ele = ele->next )
	    ++count;

	output("YYPRIVATE YY_TTYPE %s%03d[]={%2d,", row_name,
						(int)(elep-table), count);
									   /*}*/
        column = 0;
	for( ele = *elep ; ele ; ele = ele->next )
	{
	    ++Npairs;
	    output( "%3d,%-4d", ele->sym, ele->do_this );

	    if( ++column != count )
		outc( ',' );

	    if( column % 5 == 0  )
		output("\n\t\t\t        ");
	}
									/* { */
	output( "};\n" );
    }

    /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
     * Output the index array
     */

    if( make_private )
	output( "\nYYPRIVATE YY_TTYPE *%s[%d] =\n", col_name, Nstates );
    else
	output( "\nYY_TTYPE *%s[%d] =\n", col_name, Nstates );

    output( "{\n/*   0 */  " );						/* } */

    for( elep = table, i = 0 ; i < Nstates ; ++i, ++elep )
    {
	if( MEMBER(redundant, i) )
	    output( "%s%03d", row_name, (int)((ACT **)(*elep) - table) );
	else
	    output( *elep ? "%s%03d" : "  NULL" , row_name, i );

	if( i != Nstates-1 )
	    output( ", " );

	if( i==0  || (i % 8)==0 )
	    output("\n/* %3d */  ", i+1 );
    }
									/* { */
    delset( redundant );            	/* Mark redundant rows */
    output( "\n};\n");
}

#ifdef DEBUG
#ifdef __TURBOC__
#pragma warn -use
#endif

PRIVATE  char 	*strprod( prod )
PRODUCTION	*prod;
{
    /* Return a pointer to a string that holds a representation
     * of a production.
     */

    static char buf[ MAXOBUF ];
    char	*bp;
    int	   	i;

    bp  = buf;
    bp += sprintf( bp, "%s->", prod->lhs->name );

    if( prod->rhs_len <= 0 )
	bp += sprintf( bp, "<epsilon>" );
    else
	for( i = 0; i < prod->rhs_len ; ++i )
	    bp += sprintf(bp, "%s ", prod->rhs[i]->name );

    if( bp - buf >= MAXOBUF )
	error(FATAL, "Internal [strprod], buffer overflow\n" );

    *bp = '\0' ;
    return buf;
}
#endif
