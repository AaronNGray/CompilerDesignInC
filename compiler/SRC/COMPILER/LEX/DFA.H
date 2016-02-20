/*@A (C) 1992 Allen I. Holub                                                */
/*--------------------------------------------------------------
 * DFA.H: The following definitions are used in dfa.c and in
 * minimize.c to represent DFA's.
 *--------------------------------------------------------------
 */

#ifndef __DFA_H
#define __DFA_H
#include <tools/set.h>

#define DFA_MAX     254		/* Maximum number of DFA states. If this
				 * number >= 255, you'll have to change the
				 * output routines and driver. States are
				 * numbered from 0 to DFA_MAX-1
				 */
typedef	unsigned char TTYPE;	/* This is the type of the output DFA 	   */
				/* transition table (the internal one is   */
				/* an array of int). It is used only to    */
				/* figure the various table sizes printed  */
				/* by -V. Nonetheless, it should match the */
				/* definition in /lib/lex.par.		   */

#define F   	    -1		/* Marks failure states in the table.	   */
#define MAX_CHARS   128		/* Maximum width of dfa transition table.  */

typedef	int ROW[ MAX_CHARS ];   /* One full row of Dtran, which is itself */
				/* an array, DFA_MAX elements long, of    */
				/* ROWs.			    	  */
/*--------------------------------------------------------------*/
typedef struct ACCEPT
{
    char  *string;	/* Accepting string; NULL if nonaccepting.            */
    int	  anchor;	/* Anchor point, if any. Values are defined in NFA.H. */

} ACCEPT;
/*----------------------------------------------------------------------
 * External subroutines:
 */

SET  *e_closure	 P(( SET*,  char **, int *		  )); /* terp.c     */
void free_nfa	 P(( void				  )); /* terp.c	   */
SET  *move	 P(( SET*,  int 			  )); /* terp.c     */
int  nfa	 P(( char *(*)(void)			  )); /* terp.c     */
int  dfa	 P(( char *(*)(void), ROW*[], ACCEPT**	  )); /* dfa.c      */
int  min_dfa	 P(( char *(*)(void), ROW*[], ACCEPT**	  )); /* minimize.c */
int  columns	 P(( FILE*, ROW*,  int, int, char*	  )); /* columns.c  */
void cnext	 P(( FILE*, char*		   	  )); /* columns.c  */
void signon	 P(( void				  )); /* signon.c   */

							      /* print.c:   */
void pheader P(( FILE *fp,    ROW dtran[], int nrows, ACCEPT *accept));
void pdriver P(( FILE *output,             int nrows, ACCEPT *accept));

extern  void lerror  	    P((int status,char *fmt,... ));	/* lex.c   */
extern  void main	    P((int argc,char **argv	));	/* lex.c   */
extern  char *get_expr	    P(( void 			));	/* input.c */

								/* squash.c */
extern  int squash P((FILE *fp, ROW *dtran, int nrows, int ncols, char *name));
extern  void cnext P((FILE *fp, char *name));			/* squash.c */

#endif /* __DFA_H */
