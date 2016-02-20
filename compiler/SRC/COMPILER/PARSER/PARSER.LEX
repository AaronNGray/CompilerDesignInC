/*@A (C) 1992 Allen I. Holub                                                */

%{
#include <stdio.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/set.h>
#include "llout.h"

#define CREATING_LLAMA_PARSER	/* Suppress various definitions in parser.h */
#include "parser.h"		/* that conflict with LeX-generated defs.   */

/* ----------------------------------------------------------------------
 * Lexical analyzer for both llama and yacc. Note that llama doesn't support
 * %left, %right, %noassoc, or %prec. They are recognized here so that we can
 * print an error message when they're encountered. By the same token, yacc
 * ignores the %synch directive. Though all legal llama input files can be
 * processed by yacc, the reverse is not true.
 *
 * ----------------------------------------------------------------------
 * Whitespace, comments, and otherwise illegal characters must be handled
 * specially. When we're processing code blocks, we need to get at
 * the characters so that they can be passed to the output, otherwise, the
 * characters should be ignored. The ws() and nows() subroutines (at the
 * bottom of the file) switch between these behaviors by changing the value
 * if Ignore. If Ignore is true, white space is ignored.
 */

PRIVATE int   Ignore = 0;
PRIVATE int   Start_line;	/* starting line number  */

/* ----------------------------------------------------------------------
 *  Prototypes for functions at the bottom of this file:
 */

void stripcr P(( char *src ));  /* Remove carriage returns (but not	*/
			      	/* linefeeds) from src.			*/
void nows    P(( void ));	/* Ignore white space, etc 	 	*/
void ws      P(( void )); 	/* Don't ignore white space, etc 	*/
%}

c_name	[A-Za-z_][A-Za-z_0-9]*
%%

"/*"		   {	/* Absorb a comment (treat it as WHITESPACE) */

			int i;
			int start = yylineno;

			while( i = input() )
			{
		           if( i < 0 )
			   {
			      ii_unterm();
			      ii_flush(1);
			      ii_term();
			      lerror(NONFATAL,"Comment starting on line " );
			      lerror(NOHDR,   "%d too long, truncating\n",
			      						start);
			   }
			   else if( i == '*'  &&  ii_lookahead(1) == '/' )
			   {
				input();
				stripcr( yytext );

				if( Ignore ) goto end;
				else	     return WHITESPACE;
			   }
			}

			lerror(FATAL, "End of file encountered in comment\n");

		   end:;
		   }

		   /* Suck up an entire action. Handle nested braces here.
		    * This code won't work if the action is longer than the
		    * buffer length used by the input functions. If this is a
		    * problem, you have to allocate your own buffer and copy
		    * the lexeme there as it's processed (in a manner similar to
		    * the %{ processing, below). If space is really a problem,
		    * the code blocks can be copied to a temporary file and the
		    * offset to the start of the text (as returned by ftell())
		    * can be stored rather than the string itself.
		    */

\{		    {
			int  i;
			int  nestlev;		/* brace-nesting level 	  */
			int  lb1;		/* previous character     */
			int  lb2;		/* character before that  */
			int  in_string;		/* processing string constant */
			int  in_char_const;	/* processing char.  constant */
			int  in_comment;	/* processing a comment	      */

			lb1 = lb2     = 0;
			in_string     = 0;
			in_char_const = 0;
			in_comment    = 0;
			Start_line    = yylineno;

		   	for( nestlev=1; i=input(); lb2=lb1, lb1=i )
			{
			    if( lb2=='\n' && lb1=='%' && i=='%' )
			       lerror( FATAL,
				    "%%%% in code block starting on line %d\n",
				    Start_line );

			    if( i < 0 )		/* input-buffer overflow */
			    {
			      ii_unterm();
			      ii_flush(1);
			      ii_term();
			      lerror( FATAL,
				   "Code block starting on line %d too long.\n",
								    Start_line);
			    }

			    if( i == '\n' && in_string )
			    {
				lerror(WARNING,
					   "Newline in string, inserting \"\n");
				in_string = 0;
			    }

			    /* Take care of \{, "{", '{', \}, "}", '}'  */

			    if( i == '\\' )
			    {
				if( !(i = input()) )	/* discard backslash  */
				    break;
				else
				    continue;		/* and following char */
			    }

			    if( i == '"' && !(in_char_const || in_comment) )
				in_string = !in_string;

			    else if( i == '\'' && !(in_string || in_comment) )
				in_char_const = !in_char_const;

			    else if( lb1 == '/' && i == '*' && !in_string )
				in_comment = 1;

			    else if( lb1 == '*' && i == '/' && in_comment )
				in_comment = 0;

			    if( !(in_string || in_char_const || in_comment) )
			    {
				if( i == '{' )
				    ++nestlev;

				if( i == '}' && --nestlev <= 0 )
				{
				    stripcr( yytext );
				    return ACTION;
				}
			    }
			}

			lerror(FATAL, "EOF in code block starting on line %d\n",
								  Start_line );
		   }

^"%%"		   return SEPARATOR;		/* Must be anchored because  */
						/* it can appear in a printf */
						/* statement.		     */
"%{"[\s\t]*	   {
			/* Copy a code block to the output file.  */

			int c, looking_for_brace = 0;

			#undef output	       /* replace macro with function */
					       /* in main.c		      */
			if( !No_lines )
			    output( "\n#line %d \"%s\"\n",
						    yylineno, Input_file_name );

			while( c = input() )	/* while not at end of file   */
			{
			    if( c == -1 )	/* buffer is full, flush it   */
				ii_flushbuf();

			    else if( c != '\r' )
			    {
				if( looking_for_brace )	/* last char was a %  */
				{					   /*{*/
				    if( c == '}' ) break;
				    else	   output( "%%%c", c );
				    looking_for_brace = 0;
				}
				else
				{
				    if( c == '%' ) looking_for_brace = 1;
				    else	   output( "%c", c );
				}
			    }
			}
			return CODE_BLOCK;
		   }

<{c_name}>	   return FIELD;	  	/* for occs only  	   */
"%union"	   return PERCENT_UNION;  	/* for occs only  	   */
"%token"	   |
"%term"		   return TERM_SPEC;
"%type"		   return TYPE;			/* for occs only  	   */
"%synch"	   return SYNCH;		/* for llama only 	   */
"%left"		   return LEFT;			/* for occs only  	   */
"%right"	   return RIGHT;		/* for occs only  	   */
"%nonassoc"	   return NONASSOC;		/* for occs only  	   */
"%prec"		   return PREC;			/* for occs only  	   */
"%start"	   return START;		/* for error messages only */
":"		   return COLON ;
"|"		   return OR    ;
";"		   return SEMI  ;
"["	   	   return START_OPT ;
"]"		   |
"]*"	   	   return END_OPT;

[^\x00-\s%\{}[\]()*:|;,<>]+	return NAME;
\x0d				; /* discard carriage return (\r) */
[\x00-\x0c\x0e-\s]		if( !Ignore ) return WHITESPACE;

%%
/*----------------------------------------------------------------------*/

PUBLIC  void  nows()  { Ignore = 1; }     /* Ignore white space, etc.       */
PUBLIC  void  ws  ()  { Ignore = 0; }     /* Don't ignore white space, etc. */

PUBLIC  int   start_action()    /* Return starting line number of most	*/
{				/* recently read ACTION block		*/
    return Start_line;
}
/*----------------------------------------------------------------------*/

PRIVATE void stripcr( src )       /* Remove all \r's (but not \n's) from src. */
char	*src;
{
    char	*dest;
    for( dest = src ; *src ; src++ )
	if( *src != '\r' )
	    *dest++ = *src;
    *dest = '\0';
}

   /*----------------------------------------------------------------------*/
#ifdef MAIN		    /* test routine, reads lexemes and prints them */

   int No_lines	= 0;
   char *Input_file_name;
   FILE *Output = stdout;
#include <stdarg.h>

   plex( lex )
   int lex;
   {
       switch( lex )
       {
       case ACTION:	   printf("ACTION (%s)\n",	   yytext); break;
       case CODE_BLOCK:	   printf("CODE_BLOCK (%s)\n",	   yytext); break;
       case COLON:	   printf("COLON (%s)\n",	   yytext); break;
       case END_OPT:	   printf("END_OPT (%s)\n",	   yytext); break;
       case FIELD:	   printf("FIELD (%s)\n",	   yytext); break;
       case LEFT:	   printf("LEFT (%s)\n",	   yytext); break;
       case NAME:	   printf("NAME (%s)\n",	   yytext); break;
       case NONASSOC:	   printf("NONASSOC (%s)\n",	   yytext); break;
       case OR:		   printf("OR (%s)\n",		   yytext); break;
       case OTHER:	   printf("OTHER (%s)\n",	   yytext); break;
       case PERCENT_UNION: printf("PERCENT_UNION (%s)\n",  yytext); break;
       case PREC:	   printf("PREC (%s)\n",	   yytext); break;
       case RIGHT:	   printf("RIGHT (%s)\n",	   yytext); break;
       case SEMI:	   printf("SEMI (%s)\n",	   yytext); break;
       case SEPARATOR:	   printf("SEPARATOR (%s)\n",	   yytext); break;
       case START:	   printf("START (%s)\n",	   yytext); break;
       case START_OPT:	   printf("START_OPT (%s)\n",	   yytext); break;
       case SYNCH:	   printf("SYNCH (%s)\n",	   yytext); break;
       case TERM_SPEC:	   printf("TERM_SPEC (%s)\n",	   yytext); break;
       case TYPE:	   printf("TYPE (%s)\n",	   yytext); break;
       case WHITESPACE:    printf("WHITESPACE (%s)\n",	   yytext); break;
       default:		   printf("*** unknown *** (%s)\n",yytext); break;
      }
   }

   void main( argc, argv )
   int argc;
   char	**argv;
   {
       int lex;
       if( argc == 1 )
       {
   	    while( lex = yylex() )
   	        plex( lex );
       }
       else
       {
   	if( ii_newfile( Input_file_name = argv[1] ) < 0 )
   	    perror( argv[1] );
   	else
   	    while( lex = yylex() )
  	    {
   	        plex( lex );
  	    }
       }
   }

   ANSI (PUBLIC void output( char *fmt,...) )
   UNIX (PUBLIC void output( fmt )	    )	/* The real versions of these */
   UNIX (char	*fmt;			    )	/* subroutines are in main.c. */
   {						/* These stubs are here for   */
       va_list   args;				/* debugging a standalone     */
       va_start( args,   fmt );			/* scanner.                   */
       vfprintf( Output, fmt, args );
       fflush  ( Output );
   }

   ANSI (PUBLIC void lerror( int status, char *fmt,...)	)
   UNIX (PUBLIC void lerror( status, fmt )		)
   UNIX (int	status;					)
   UNIX (char	*fmt;					)
   {
       va_list   args;
       va_start( args,   fmt );
       vfprintf( stderr, fmt, args );
       fflush  ( stderr );
       if( status == FATAL )
  	   exit(1);
   }
#endif
