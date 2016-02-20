/*@A (C) 1992 Allen I. Holub                                                */
/* These functions are all public functions from the LLama/occs output file.
 * Lex-output-file functions, etc., are also provided
 */

#include <tools/debug.h>	/* for P() macro */

void	yycode	    P(( char *fmt, ...	));
void	yydata	    P(( char *fmt, ...	));
void	yybss 	    P(( char *fmt, ...	));
void 	yyerror     P(( char *fmt, ...  ));
void 	yycomment   P(( char *fmt, ...  ));
int     yyparse     P(( void 		));
int     yylex       P(( void 		));

extern FILE	*yycodeout;
extern FILE	*yydataout;
extern FILE	*yybssout;

extern char *yytext;
extern int   yyleng;
extern int   yylineno;
