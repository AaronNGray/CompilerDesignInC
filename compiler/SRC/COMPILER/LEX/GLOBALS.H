/*@A (C) 1992 Allen I. Holub                                                */
/* 			GLOBALS.H: Global variables shared between modules */
#ifdef ALLOC
#	define CLASS

#	define I(x) x
#else

#	define CLASS extern
#	define I(x)
#endif
#define MAXINP	2048			  /* Maximum rule size		   */

CLASS int  Verbose	   I( = 0 );	  /* Print statistics		   */
CLASS int  No_lines        I( = 0 );	  /* Suppress #line directives	   */
CLASS int  Unix		   I( = 0 );	  /* Use UNIX-style newlines       */
CLASS int  Public	   I( = 0 );	  /* Make static symbols public    */
CLASS char *Template       I(="lex.par"); /* State-machine driver template */
CLASS int  Actual_lineno   I( = 1 );	  /* Current input line number	   */
CLASS int  Lineno	   I( = 1 );	  /* Line number of first line of  */
					  /* a multiple-line rule.	   */
CLASS char Input_buf[MAXINP];		  /* Line buffer for input	   */
CLASS char *Input_file_name;		  /* Input file name (for #line)   */
CLASS FILE *Ifile;			  /* Input stream.		   */
CLASS FILE *Ofile;			  /* Output stream.		   */
#undef CLASS
#undef I
