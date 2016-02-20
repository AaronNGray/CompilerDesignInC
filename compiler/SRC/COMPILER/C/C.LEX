/*@A (C) 1992 Allen I. Holub                                                */

/* Lexical analyzer specification for C. This is a somewhat extended version
 * of the one in Appendix D. The main difference is that it passes attributes
 * for some tokens back to the parser, using the yylval mechanism to push
 * the attribute onto the value stack.
 */

%{
#include <stdio.h>
#include <string.h>
#include <search.h>	 /* Function prototype for bsearch(). 	*/
#include <tools/debug.h> /* Needed by symtab.h.			*/
#include <tools/hash.h>	 /* Needed by symtab.h.			*/
#include <tools/l.h>
#include <tools/compiler.h>
#include "yyout.h"	/* Token defs. created by occs. Yacc uses y.tab.h. */
#include "symtab.h"	/* Needed to pass attributes to parser.		   */
#include "value.h"	/* ditto					   */

extern union {		      /* This definition must duplicate the %union */
    char	*p_char;      /* in c.y.				     */
    symbol	*p_sym;
    link	*p_link;
    structdef	*p_sdef;
    specifier	*p_spec;
    value	*p_value;
    int		integer;
    int		ascii;
}
yylval;				/* Declared by occs in yyout.c. */

extern FILE *yycodeout;		/* Declared by occs in yyout.c	*/

static int id_or_keyword P(( char *lex ));	/* declared, below */
/*----------------------------------------------------------------------*/

#define YYERROR	yyerror	/* Forces LeX-generated error messages to be     */
			/* output in an occs window if debugging mode    */
			/* is enabled.  Does nothing in UNIX lex. Remove */
			/* this definition if you aren't using occs -d.  */

/*----------------------------------------------------------------------*/
%}

let     [_a-zA-Z]	/* Letter 					*/
alnum   [_a-zA-Z0-9]	/* Alphanumeric character			*/
h       [0-9a-fA-F]	/* Hexadecimal digit				*/
o       [0-7]		/* Octal digit					*/
d       [0-9]		/* Decimal digit				*/
suffix  [UuLl]		/* Suffix in integral numeric constant		*/
white   [\x00-\x09\x0b\s]      /* White space: all control chars but \n */

%%
"/*"                {
                        int i;

                        while( i = ii_input() )
                        {
                           if( i < 0 )
                              ii_flushbuf();   		/* Discard lexeme. */

                           else if( i == '*'  &&  ii_lookahead(1) == '/' )
                           {
                                ii_input();
                                break;          	/* Recognized comment.*/
                           }
                        }

                        if( i == 0 )
                           yyerror( "End of file in comment\n" );
                    }

\"(\\.|[^\"])*\"    { return STRING; }

\"(\\.|[^\"])*\n    yyerror("Adding missing \" to string constant\n");
		    yymore();

'.'                                  |
'\\.'                         	     |
'\\{o}({o}{o}?)?'		     |
'\\x{h}({h}{h}?)?'		     |
0{o}*{suffix}?                	     |
0x{h}+{suffix}?               	     |
[1-9]{d}*{suffix}?                   return ICON ;

({d}+|{d}+\.{d}*|{d}*\.{d}+)([eE][\-+]?{d}+)?[fF]?   return FCON ;

"("                     return LP;
")"                     return RP;
"{"                     return LC;
"}"                     return RC;
"["                     return LB;
"]"                     return RB;

"->"                    |
"."                     yylval.ascii = *yytext;
			return STRUCTOP;

"++"                    |
"--"                    yylval.ascii = *yytext;
			return INCOP;

[~!]                    yylval.ascii = *yytext;
			return UNOP;

"*"                     return STAR;

[/%]                    yylval.ascii = *yytext;
			return DIVOP;

"+"                     return PLUS;
"-"                     return MINUS;

<<|>>                   yylval.ascii = *yytext;
		        return SHIFTOP;

[<>]=?                  yylval.ascii = yytext[1] ? (yytext[0]=='>' ? 'G' : 'L')
						 : (yytext[0]		      );
			return RELOP;

[!=]=                   yylval.ascii = *yytext;
                   	return EQUOP;

[*/%+\-&|^]=		|
(<<|>>)=		yylval.ascii = *yytext;
			return ASSIGNOP;

"="                     return EQUAL;
"&"                     return AND;
"^"                     return XOR;
"|"                     return OR;
"&&"                    return ANDAND;
"||"                    return OROR;
"?"                     return QUEST;
":"                     return COLON;
","                     return COMMA;
";"                     return SEMI;
"..."			return ELLIPSIS;

{let}{alnum}*           return id_or_keyword( yytext );

\n		     fprintf(yycodeout, "\t\t\t\t\t\t\t\t\t/*%d*/\n", yylineno);
{white}+             ;	/* ignore other white space */
.                    yyerror( "Illegal character <%s>\n", yytext );
%%

/*------------------------------------------------------------------*/
typedef struct		/* Routines to recognize keywords. A table	*/
{			/* lookup is used for this purpose in order to	*/
    char  *name;	/* minimize the number of states in the FSM. A	*/
    int   val;		/* KWORD is a single table entry.		*/
}
KWORD;

KWORD  Ktab[] =			/* Alphabetic keywords	*/
{
    { "auto",     CLASS         },
    { "break",    BREAK         },
    { "case",     CASE          },
    { "char",     TYPE          },
    { "continue", CONTINUE      },
    { "default",  DEFAULT       },
    { "do",       DO            },
    { "double",   TYPE          },
    { "else",     ELSE          },
    { "enum",     ENUM          },
    { "extern",   CLASS         },
    { "float",    TYPE          },
    { "for",      FOR           },
    { "goto",     GOTO          },
    { "if",       IF            },
    { "int",      TYPE          },
    { "long",     TYPE          },
    { "register", CLASS         },
    { "return",   RETURN        },
    { "short",    TYPE          },
    { "sizeof",   SIZEOF        },
    { "static",   CLASS         },
    { "struct",   STRUCT        },
    { "switch",   SWITCH        },
    { "typedef",  CLASS         },
    { "union",    STRUCT        },
    { "unsigned", TYPE          },
    { "void",     TYPE          },
    { "while",    WHILE         }
};

static int cmp( a, b )
KWORD   *a, *b;
{
    return strcmp( a->name, b->name );
}

static int id_or_keyword( lex )	/* Do a binary search for a */
char    *lex;			/* possible keyword in Ktab */
{				/* Return the token if it's */
    KWORD         *p;		/* in the table, NAME       */
    KWORD         dummy;	/* otherwise.		    */

    dummy.name = lex;
    p = (KWORD *) bsearch( &dummy, Ktab, sizeof(Ktab)/sizeof(KWORD),
								sizeof(KWORD),
	BCC ((int _Cdecl(*)   (const void _FAR *, const void _FAR *)))
	MSC ((int  (__cdecl *)(const void *,      const void      *))) cmp );

    if( p )						/* It's a keyword. */
    {
	yylval.ascii = *yytext;
	return p->val;
    }
    else if( yylval.p_sym = (symbol *) findsym( Symbol_tab, yytext ) )
	return (yylval.p_sym->type->tdef) ? TTYPE : NAME ;
    else
	return NAME;
}
#ifdef TEST_LEX
ptok( int tok  )
{
	switch( tok )
	{
	case STRING:	printf("STRING (%s)\n",	yytext );	break;
	case ICON:	printf("ICON (%s)\n",	yytext );	break;
	case FCON:	printf("FCON (%s)\n",	yytext );	break;
	case TYPE:	printf("TYPE (%s)\n",	yytext );	break;
	case STRUCT:	printf("STRUCT (%s)\n",	yytext );	break;
	case ENUM:	printf("ENUM (%s)\n",	yytext );	break;
	case RETURN:	printf("RETURN (%s)\n",	yytext );	break;
	case GOTO:	printf("GOTO (%s)\n",	yytext );	break;
	case IF:	printf("IF (%s)\n",	yytext );	break;
	case ELSE:	printf("ELSE (%s)\n",	yytext );	break;
	case SWITCH:	printf("SWITCH (%s)\n",	yytext );	break;
	case CASE:	printf("CASE (%s)\n",	yytext );	break;
	case DEFAULT:	printf("DEFAULT (%s)\n",yytext );	break;
	case BREAK:	printf("BREAK (%s)\n",	yytext );	break;
	case CONTINUE:	printf("CONTINUE (%s)\n",yytext );	break;
	case WHILE:	printf("WHILE (%s)\n",	yytext );	break;
	case DO:	printf("DO (%s)\n",	yytext );	break;
	case FOR:	printf("FOR (%s)\n",	yytext );	break;
	case LC:	printf("LC (%s)\n",	yytext );	break;
	case RC:	printf("RC (%s)\n",	yytext );	break;
	case SEMI:	printf("SEMI (%s)\n",	yytext );	break;
	case ELLIPSIS:	printf("ELLIPSIS (%s)\n",yytext );	break;
	case COMMA:	printf("COMMA (%s)\n",	yytext );	break;
	case EQUAL:	printf("EQUAL (%s)\n",	yytext );	break;
	case ASSIGNOP:	printf("ASSIGNOP (%s)\n",yytext );	break;
	case QUEST:	printf("QUEST (%s)\n",	yytext );	break;
	case COLON:	printf("COLON (%s)\n",	yytext );	break;
	case OROR:	printf("OROR (%s)\n",	yytext );	break;
	case ANDAND:	printf("ANDAND (%s)\n",	yytext );	break;
	case OR:	printf("OR (%s)\n",	yytext );	break;
	case XOR:	printf("XOR (%s)\n",	yytext );	break;
	case AND:	printf("AND (%s)\n",	yytext );	break;
	case EQUOP:	printf("EQUOP (%s)\n",	yytext );	break;
	case RELOP:	printf("RELOP (%s)\n",	yytext );	break;
	case SHIFTOP:	printf("SHIFTOP (%s)\n",yytext );	break;
	case PLUS:	printf("PLUS (%s)\n",	yytext );	break;
	case MINUS:	printf("MINUS (%s)\n",	yytext );	break;
	case STAR:	printf("STAR (%s)\n",	yytext );	break;
	case DIVOP:	printf("DIVOP (%s)\n",	yytext );	break;
	case SIZEOF:	printf("SIZEOF (%s)\n",	yytext );	break;
	case UNOP:	printf("UNOP (%s)\n",	yytext );	break;
	case INCOP:	printf("INCOP (%s)\n",	yytext );	break;
	case LB:	printf("LB (%s)\n",	yytext );	break;
	case RB:	printf("RB (%s)\n",	yytext );	break;
	case LP:	printf("LP (%s)\n",	yytext );	break;
	case RP:	printf("RP (%s)\n",	yytext );	break;
	case STRUCTOP:	printf("STRUCTOP (%s)\n",yytext );	break;
	case TTYPE:	printf("TTYPE (%s)\n",	yytext );	break;
	case CLASS:	printf("CLASS (%s)\n",	yytext );	break;
	case NAME:	printf("NAME (%s)\n",	yytext );	break;
	}
}
#endif
