/*@A (C) 1992 Allen I. Holub                                                */

%term  NUM_OR_ID	/*  a number or identifier		*/

%left  PLUS 		/*  +		(lowest precedence)	*/
%left  STAR		/*  *					*/
%left  LP RP		/*  ( )		(highest precedence)	*/

%{
#include <stdio.h>
#include <ctype.h>
#include <malloc.h>
#include <tools/debug.h>
#include <tools/stack.h>

extern char *yytext;

stack_dcl( Namepool, char*, 10 );	   /* Stack of 10 temporary-var names */
#define freename(x) push( Namepool, (x) )  /* Release a temporary variable    */
#define	getname()   pop (  Namepool     )  /* Allocate an temporary variable  */

typedef char		*stype;	     /* Value stack is stack of char pointers */

#define YYSTYPE 	stype

#define YYSHIFTACT(tos) (*tos="")		     /* Shift a null string */
%}

%%
/* A small expression grammar that recognizes numbers, names, addition (+),
 * multiplication (*), and parentheses. Expressions associate left to right
 * unless parentheses force it to go otherwise. * is higher precedence than +.
 */

s	: e ;

e	: e PLUS t   { yycode("%s += %s\n", $1, $3); freename($3); }
	| t 	     /* $$ = $1	 */
	;

t	: t STAR f   { yycode("%s *= %s\n", $1, $3); freename($3); }
	| f 	     /* $$ = $1	 */
	;

f	: LP e RP    { $$ = $2;	}
  	| NUM_OR_ID  {
			/* Copy operand to a temporary. Note that I'm adding an
			 * underscore to external names so that they can't con-
			 * flict with the compiler-generated temporary names
			 * (t0, t1, etc.).
			 */

			yycode("%s = %s%s\n", $$ = getname(),
					      isdigit(*yytext) ? "" : "_",
					      yytext );
		     }
	;
%%
/*----------------------------------------------------------------------*/

char	*yypstk( vptr, dptr )
char	**vptr;			/* Value-stack pointer 		   */
char	*dptr;			/* Symbol-stack pointer (not used) */
{
    /* Yypstk is used by the debugging routines. It is passed a pointer to a
     * value-stack item and should return a string representing that item. Since
     * the current value stack is a stack of string pointers, all it has to do
     * is dereference one level of indirection.
     */

    return *vptr ? *vptr : "-" ;
}

/*----------------------------------------------------------------------*/

yy_init_occs()
{
    /* Called by yyparse just before it starts parsing. Initialize the
     * temporary-variable-name stack and output declarations for the variables.
     */

    push( Namepool, "t9" ); push( Namepool, "t8" ); push( Namepool, "t7" );
    push( Namepool, "t6" ); push( Namepool, "t5" ); push( Namepool, "t4" );
    push( Namepool, "t3" ); push( Namepool, "t2" ); push( Namepool, "t1" );
    push( Namepool, "t0" );

    yycode( "public word t0, t1, t2, t3, t4;\n" );
    yycode( "public word t5, t6, t7, t8, t9;\n" );
}

/*----------------------------------------------------------------------*/

main( argc, argv )
char	**argv;
{
    yy_get_args( argc, argv );

    if( argc < 2 )
	ferr("Need file name\n");

    else if( ii_newfile(argv[1]) < 0 )
	ferr( "Can't open %s\n", argv[1] );

    yyparse();
    exit( 0 );
}

