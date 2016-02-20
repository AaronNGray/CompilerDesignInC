/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __NFA_H
#define __NFA_H
/*--------------------------------------------------------------
 * Nfa state:
 */

typedef struct NFA
{
    int	        edge;	    /* Label for edge: character, CCL, EMPTY, or */
			    /* EPSILON.			 		 */
    SET	        *bitset;    /* Set to store character classes. 	 	 */
    struct NFA  *next;	    /* Next state (or NULL if none)		 */
    struct NFA  *next2;     /* Another next state if edge==EPSILON	 */
			    /* NULL of this state isn't used		 */
    char        *accept;    /* NULL if not an accepting state, else	 */
			    /* a pointer to the action string		 */
    int		anchor;	    /* Says whether pattern is anchored and, if  */
			    /* so, where (uses #defines above).		 */
} NFA;


#define EPSILON  -1		/* Non-character values of NFA.edge	 */
#define CCL	 -2
#define EMPTY	 -3

					/* Values of the anchor field:	*/
#define NONE	0  			/*   Not anchored	     	*/
#define START   1  			/*   Anchored at start of line 	*/
#define END     2  			/*   Anchored at end of line   	*/
#define BOTH    ( START | END ) 	/*   Anchored in both places	*/
#define NFA_MAX	768		/* Maximum number of NFA states in a	    */
				/* single machine. NFA_MAX * sizeof(NFA)    */
				/* can't exceed 64K.			    */
#define STR_MAX (10 * 1024)	/* Total space that can be used by the 	    */
				/* accept strings.			    */

void	print_nfa	P(( NFA*, int, NFA*	));	/* printnfa.c	*/
void	new_macro	P((char *def		));	/* nfa.c	*/
void	printmacs	P((void 		));
NFA	*thompson	P(( char *( *input_function )(void),\
				    int	*max_state, NFA	**start_state));
#endif /* __NFA_H */
