/*@A (C) 1992 Allen I. Holub                                                */
/*	PARSER.H	This file contains those #defines, etc., that
 *			are used by both llama and yacc. There's also
 *			a yacc.h file that's used only by the yacc code.
 */

#ifndef __PARSER_H
#define __PARSER_H

#ifndef NULL
#include <stdio.h>
#endif
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/debug.h>

#ifdef LLAMA
#define LL(x) x
#define OX(x)
#else
#define LL(x)
#define OX(x) x
#endif

/*----------------------------------------------------------------------
 * Various error exit stati. Note that other errors found while parsing cause
 * llama to exit with a status equal to the number of errors (or zero if
 * there are no errors).
 */

#define EXIT_ILLEGAL_ARG	255	/* Illegal command-line switch	    */
#define EXIT_TOO_MANY		254	/* Too many command-line args	    */
#define EXIT_NO_DRIVER		253	/* Can't find llama.par		    */
#define EXIT_OTHER		252	/* Other error (syntax error, etc.) */
#define EXIT_USR_ABRT		251	/* Ctrl-Break			    */

#define MAXNAME     32	/* Maximum length of a terminal or nonterminal name   */
#ifdef DEMO
#define MAXPROD    8	/* Maximum number of productions  		      */
#else
#define MAXPROD	   512	/* Maximum number of productions in the input grammar */

#define MINTERM	     1	/* Token values assigned to terminals start here    */
#define MINNONTERM 256	/* nonterminals start here			    */
#define MINACT	   512	/* acts start here				    */

				/* Maximum numeric values used for terminals
				 * and nonterminals (MAXTERM and MINTERM), as
				 * well as the maximum number of terminals and
				 * nonterminals (NUMTERMS and NUMNONTERMS).
				 * Finally, USED_TERMS and USED_NONTERMS are
				 * the number of these actually in use (i.e.
				 * were declared in the input file).
				 */

#define MAXTERM     (MINNONTERM -2)
#define MAXNONTERM  (MINACT     -1)

#define NUMTERMS    ((MAXTERM-MINTERM) +1)
#define NUMNONTERMS ((MAXNONTERM-MINNONTERM)+1)

#define USED_TERMS	((Cur_term    - MINTERM)    +1)
#define USED_NONTERMS	((Cur_nonterm - MINNONTERM) +1)

				/* These macros evaluate to true if x represents
				 * a terminal (ISTERM), nonterminal (ISNONTERM)
				 * or action (ISACT)
				 */

#define ISTERM(x)    ((x) && (MINTERM    <= (x)->val && (x)->val <= MAXTERM   ))
#define ISNONTERM(x) ((x) && (MINNONTERM <= (x)->val && (x)->val <= MAXNONTERM))
#define ISACT(x)     ((x) && (MINACT     <= (x)->val 		  	      ))

			         /* Epsilon's value is one more than the largest
			 	  * terminal actually used. We can get away with
				  * this only because EPSILON is not used until
				  * after all the terminals have been entered
				  * into the symbol table.
			 	  */
#define EPSILON  (Cur_term+1)
				  /* The following macros are used to adjust the
				   * nonterminal values so that the smallest
				   * nonterminal is zero. (You need to do this
				   * when you output the tables). ADJ_VAL does
				   * the adjustment, UNADJ_VAL translates the
				   * adjust value back to the original value.
				   */

#define ADJ_VAL(x)   ((x)-MINNONTERM )
#define UNADJ_VAL(x) ((x)+MINNONTERM )

/*----------------------------------------------------------------------*/

#define NONFATAL	0	/* Values to pass to error() and lerror(), */
#define FATAL		1	/* defined in main.c.			   */
#define WARNING		2
#define NOHDR		3

#define DOLLAR_DOLLAR	((unsigned)~0 >>1)	/* Passed to do_dollar() to */
						/* indicate that $$ is to   */
						/* be processed.	    */

/*----------------------------------------------------------------------*/

#endif
#ifdef LLAMA				/* Various file names:		      */
#    define TOKEN_FILE	"llout.h"	/* output file for token #defines     */
#    define PARSE_FILE	"llout.c"	/* output file for parser	      */
#    define SYM_FILE	"llout.sym"	/* output file for symbol table	      */
#    define DOC_FILE	"llout.doc"	/* LALR(1) State machine description  */
#    define DEF_EXT	"lma"		/* foo.lma is default input extension */
#    define PAR_TEMPL	"llama.par" 	/* template file for parser	      */
#    define PROG_NAME   "llama"
#else
#    define TOKEN_FILE	"yyout.h"	/* output file for token #defines    */
#    define PARSE_FILE	"yyout.c"	/* output file for parser	     */
#    define ACT_FILE	"yyact.c"	/* Used for output if -a specified   */
#    define TAB_FILE	"yyoutab.c"	/* output file for parser tables (-T)*/
#    define SYM_FILE	"yyout.sym"	/* output file for symbol table	     */
#    define DOC_FILE	"yyout.doc"	/* LALR(1) State machine description */
#    define DEF_EXT	"ox"		/* foo.ox is default input extension */
#    define PAR_TEMPL	"occs.par" 	/* template file for PARSE_FILE      */
#    define ACT_TEMPL	"occs-act.par" 	/* template file for ACT_FILE	     */
#    define PROG_NAME   "occs"
#endif
/* The following are used to define types of the OUTPUT transition tables. The
 * ifndef takes care of compiling the llama output file that will be used to
 * recreate the llama input file. We must let the llama-generated definitions
 * take precedence over the the default ones in parser.h in this case.
 */
#ifndef CREATING_LLAMA_PARSER
    LL(	typedef unsigned char	YY_TTYPE ;	)
    OX(	typedef int		YY_TTYPE ;	)

#endif
/* SYMBOL structure (used for symbol table). Note that the name itself */
/* is kept by the symbol-table entry maintained by the hash function.  */

#define NAME_MAX 32				/* Max name length + 1 */

typedef struct _symbol_
{
    char	   name  [ NAME_MAX ];		/* symbol name. Must be first */
    char	   field [ NAME_MAX ];		/* %type <field>	      */
    unsigned	   val;				/* numeric value of symbol    */
    unsigned	   used;			/* symbol used on an rhs      */
    unsigned	   set;				/* symbol defined             */
    unsigned	   lineno;			/* input line num. of string  */
    char  	   *string;			/* code for actions.          */
    struct  _prod_ *productions;		/* right-hand sides if nonterm*/
    SET		   *first;			/* FIRST set		      */
LL( SET		   *follow; )			/* FOLLOW set		      */
} SYMBOL;


#define NULLABLE(sym) ( ISNONTERM(sym) && MEMBER((sym)->first, EPSILON) )
/*----------------------------------------------------------------------*/
/* PRODUCTION Structure.  Represents right-hand sides.  		*/

#define MAXRHS	 31	/* Maximum number of objects on a right-hand side   */
#define RHSBITS  5	/* Number of bits required to hold MAXRHS	    */

typedef struct _prod_
{
    unsigned  	  num;			    /* production number	      */
    SYMBOL	  *rhs[ MAXRHS + 1];	    /* Tokenized right-hand side      */
    SYMBOL	  *lhs;			    /* Left-hand side		      */
    unsigned char rhs_len;		    /* # of elements in rhs[] array   */
    unsigned char non_acts;		    /*        "  that are not actions */
    SET		  *select;		    /* LL(1) select set	              */
    struct _prod_ *next;		    /* pointer to next production     */
					    /*       for this left-hand side. */
    OX( int	  prec; )	            /* Relative precedence 	      */

} PRODUCTION;

  /*----------------------------------------------------------------------
   * Things that are unique to OCCS:
   */

#ifdef OCCS

typedef struct _prectab_
{
    unsigned char level;  /* Relative precedence, 0=none, 1=lowest	 */
    unsigned char assoc;  /* associativity: 'l'=left,'r'=right,'\0'=none */

} PREC_TAB;


#define DEF_FIELD "yy_def"	/* Field name for default field in a	*/
				/* %union.				*/
#endif
#ifdef ALLOCATE			  /* If ALLOCATE is true, allocate space and */
#	define CLASS		  /* activate the initializer, otherwise the */
#	define I(x) x		  /* storage class is extern and the 	     */
#else				  /* initializer is gobbled up.		     */
#	define CLASS extern
#	define I(x)
#endif					      /* The following are set in     */
					      /* main.c, mostly by command-   */
					      /* line switches.               */
CLASS int  Debug	    I( = 0	   ); /* True for debug diagnostics   */
CLASS char *Input_file_name I( = "console" ); /* Input file name	      */
CLASS int  Make_actions     I( = 1	   ); /* ==0 if -p on command line    */
CLASS int  Make_parser      I( = 1	   ); /* ==0 if -a on command line    */
CLASS int  Make_yyoutab     I( = 0	   ); /* ==1 if -T on command line    */
CLASS int  No_lines	    I( = 0	   ); /* Suppress #lines in output    */
CLASS int  No_warnings	    I( = 0	   ); /* Suppress warnings if true    */
CLASS FILE *Output          I( = stdout	   ); /* Output stream.		      */
CLASS int  Public	    I( = 0         ); /* Make static symbols public   */
CLASS int  Symbols	    I( = 0	   ); /* Generate symbol table.	      */
CLASS int  Threshold 	    I( = 4	   ); /* Compression threshold	      */
CLASS int  Uncompressed     I( = 0	   ); /* Don't compress tables	      */
CLASS int  Use_stdout       I( = 0         ); /* -t specified on command line */
CLASS int  Verbose    	    I( = 0	   ); /* Verbose-mode output (1 for   */
					      /* -v and 2 for -V)	      */

CLASS SYMBOL   *Terms[ MINACT ];    /* This array is indexed by terminal or
				     * nonterminal value and evaluates to a
				     * pointer to the equivalent symbol-table
				     * entry.
				     */

OX( CLASS PREC_TAB Precedence[ MINNONTERM ]; )  /* Used only by occs. Holds   */
						/* relative precedence and    */
						/* associativity information  */
						/* for both terminals and     */
						/* nonterminals.	      */

LL( CLASS  SET	*Synch;	)	    	    /* Error-recovery synchronization */
					    /* set (specified with %synch).   */
				    	    /* Synch is initialized in acts.c */
					    /* It is used only by llama.      */
CLASS  char	*Template  I(=PAR_TEMPL);   /* Template file for the parser;  */
					    /* can be modified in main.c      */
CLASS HASH_TAB  *Symtab; 	    	    /* The symbol table itself        */
					    /* (initialized in yyact.c)	      */
CLASS SYMBOL	*Goal_symbol I(=NULL);      /* Pointer to symbol-table entry  */
					    /* for the start (goal) symbol    */
				    	          /* The following are used   */
						  /* by the acts [in llact.c] */
CLASS int      Cur_term	       I(= MINTERM-1   ); /* Current terminal	      */
CLASS int      Cur_nonterm     I(= MINNONTERM-1); /*  "      nonterminal      */
CLASS int      Cur_act 	       I(= MINACT-1    ); /*  "      action	      */
CLASS int      Num_productions I( = 0          ); /* Number of productions    */

#undef CLASS
#undef I
/*----------------------------------------------------------------------*/

#define  outc(c)  putc(c,Output);	/* Character routine to complement */
					/* output() in main.c		   */

   /*--------------------------------------------------------------------------
    * External Variables and Subroutines
    */

   extern int  yynerrs;	/* yynerrs is the total error count. It is created
   			 * automatically by llama, so there is one definition
   			 * of it in the llama output file. There's also a
   			 * definition of in in the recursive-descent parser,
   			 * llpar.c.
   			 */
   								/* Defined in:*/

  int  start_action 	P((void));  	/* from lexical analyzer (parser.lex)*/

  void	code_header	P(( void ));				/* yydriver.c */
  void	code_header	P(( void ));				/* lldriver.c */
  char  *do_dollar  	P(( int num, int rhs_size, int lineno,\
  			       PRODUCTION *prod, char *fname));	/* main.c     */
  void  document	P(( char *fmt, ... ));			/* main.c     */
  void  document_to	P(( FILE *fp ));			/* main.c     */
  void	driver		P(( void ));				/* lldriver.c */
  void	driver		P(( void ));				/* yydriver.c */
  void	error	  	P(( int fatal, char *fmt, ... ));	/* main.c     */
  void	file_header	P(( void ));				/* yydriver.c */
  void	file_header	P(( void ));				/* lldriver   */
  void	first		P(( void ));				/* first.c    */
  int	first_rhs	P(( SET *dest, SYMBOL **rhs, int len ));/* first.c    */
  void	follow		P(( void ));				/* follow.c   */
  void	lerror	  	P(( int fatal, char *fmt, ... ));	/* main.c     */
  int	lr_conflicts	P(( FILE *fp ));			/* yystate.c  */
  void	lr_stats	P(( FILE *fp ));			/* yystate.c  */
  void	make_parse_tables P(( void ));				/* yystate.c  */
  void	make_token_file P(( void ));				/* stok.c     */
  void	make_yy_stok    P(( void ));				/* stok.c     */
  char  *	open_errmsg	P(( void ));			/* main.c     */
  void	output		P(( char *fmt, ... ));			/* main.c     */
  void	patch		P(( void ));				/* yypatch.c */
  void	select		P(( void ));				/* llselect   */
  void	signon		P(( void ));				/* signon.c   */
  void	tables		P(( void ));				/* llcode.c   */
  void	tables		P(( void )); 				/* yycode.c   */
  int 	yyparse		P(( void ));				/* llpar.c    */

  char	*production_str	P((struct _prod_ *prod));		/* acts.c */
  int	problems	P((void ));				/* acts.c */
  SYMBOL	*make_term	P((char *name));			/* acts.c */
  SYMBOL	*new_nonterm	P((char *name,int is_lhs));		/* acts.c */
  void	add_synch	P((char *name));			/* acts.c */
  void	add_to_rhs	P((char *object,int is_an_action));	/* acts.c */
  void	end_opt		P((char *lex));				/* acts.c */
  int	fields_active	P(( void ));				/* acts.c */
  void	first_sym	P((void ));				/* acts.c */
  void	init_acts	P((void ));				/* acts.c */
  void	new_field	P((char *field_name));			/* acts.c */
  void	new_lev		P((int how));				/* acts.c */
  void	new_rhs		P((void ));				/* acts.c */
  void	pact		P((SYMBOL *sym,FILE *stream));		/* acts.c */
  void	pnonterm	P((SYMBOL *sym,FILE *stream));		/* acts.c */
  void	prec		P((char *name));			/* acts.c */
  void	prec_list	P((char *name));			/* acts.c */
  void	print_symbols	P((FILE *stream));			/* acts.c */
  void	print_tok	P((FILE *stream,char *format,int arg));	/* acts.c */
  void	pterm		P((SYMBOL *sym,FILE *stream));		/* acts.c */
  void	start_opt	P((char *lex));				/* acts.c */
  void	union_def	P((char *action));			/* acts.c */

#endif		/* for #ifndef __PARSER_H at top of file */
