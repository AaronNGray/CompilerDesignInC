/*@A (C) 1992 Allen I. Holub                                                */




   /* ANSI C, more or less.
    *
    * Note that I've restricted myself to yacc-compatible constructions here.
    * Negative attributes would be very handy in places and it's difficult
    * not to use them.
    */

%{
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <tools/debug.h>	/* Misc. macros. (see Appendix A)	 */
#include <tools/hash.h>		/* Hash-table support. (see Appendix A)	 */
#include <tools/compiler.h> 	/* Prototypes for comp.lib functions.	 */
#include <tools/l.h>	    	/* Prototypes for l.lib functions.	 */
#include <tools/occs.h>	    	/* Prototypes for LeX/occs-generated stuff   */
#include <tools/c-code.h>    	/* Virtual-machine definitions.		 */

#ifdef __TURBOC__		/* Borland			*/
#include <dir.h>		/* for mktemp() prototype	*/
#endif
#ifdef MSDOS			/* Microsoft.			*/
#include <io.h>			/* for mktemp() prototype	*/
#endif

#ifdef YYACTION
#define ALLOC		/* Define ALLOC to create symbol table in symtab.h.  */
#endif

#include "symtab.h"	/* Definitions for the symbol-table.	     	     */
#include "value.h"	/* Definitions used for expression processing.	     */
#include "label.h"	/* Prefixes used for compiler-generated labels.	     */
#include "switch.h"	/* Definitions used for switch processing.	     */
#include "proto.h"	/* Function prototypes for all .c files used by the  */
			/* parser. It is not printed anywhere in the book,   */
			/* but is included on the distribution disk.	     */

PRIVATE void clean_up P((void));
%}
/*----------------------------------------------------------------------*/

%union {
    char	*p_char;
    symbol	*p_sym;
    link	*p_link;
    structdef	*p_sdef;
    specifier	*p_spec;
    value	*p_val;
    int		num;		/* Make short if sizeof(int) > sizeof(int*) */
    int		ascii;
}

/*----------------------------------------------------------------------*/

%term	STRING		/* String constant.				 */
%term	ICON		/* Integer or long constant including '\t', etc. */
%term	FCON		/* Floating-point constant.			 */

%term	TYPE	    	/* int char long float double signed unsigned short */
			/* const volatile void				    */
%term	<ascii> STRUCT	/* struct union			     		    */
%term	ENUM		/* enum				     		    */

%term	RETURN GOTO
%term	IF ELSE
%term	SWITCH CASE DEFAULT
%term	BREAK CONTINUE
%term	WHILE DO FOR
%term	LC RC		/* 	{ }	*/
%term	SEMI		/* 	 ;	*/
%term	ELLIPSIS	/*	...	*/

/* The attributes used below tend to be the sensible thing. For example, the
 * ASSIGNOP attribute is the operator component of the lexeme; most other
 * attributes are the first character of the lexeme. Exceptions are as follows:
 *	token	  attribute
 *	RELOP >	    '>'
 *	RELOP <     '<'
 *	RELOP >=    'G'
 *	RELOP <=    'L'
 */

%left	COMMA			     /*	,			     	    */
%right	EQUAL <ascii> ASSIGNOP 	     /* =   *= /= %= += -= <<= >>= &= |= ^= */
%right	QUEST COLON		     /* 	? :			    */
%left	OROR			     /*	||				    */
%left	ANDAND			     /*	&&				    */
%left	OR			     /*	|				    */
%left   XOR			     /*	^				    */
%left	AND			     /*	&				    */
%left	<ascii> EQUOP		     /*	==  !=				    */
%left	<ascii> RELOP		     /*	<=  >= <  >			    */
%left	<ascii> SHIFTOP		     /*	>> <<				    */
%left	PLUS  MINUS		     /*	+ -				    */
%left	STAR  <ascii> DIVOP	     /*	*  /   %			    */
%right	SIZEOF <ascii> UNOP INCOP    /*        sizeof     ! ~     ++ --     */
%left	LB RB LP RP <ascii> STRUCTOP /*	[ ] ( )  . ->			    */


			/* These attributes are shifted by the scanner.   */
%term <p_sym> TTYPE	/* Name of a type created with a previous typedef.*/
			/* Attribute is a pointer to the symbol table     */
			/* entry for that typedef.			  */
%nonassoc <ascii> CLASS	/* extern register auto static typedef. Attribute */
			/* is the first character of the lexeme.	  */
%nonassoc <p_sym> NAME	/* Identifier or typedef name. Attribute is NULL  */
			/* if the symbol doesn't exist, a pointer to the  */
			/* associated "symbol" structure, otherwise.	  */

%nonassoc ELSE		/* This gives a high precedence to ELSE to suppress
			 * the shift/reduce conflict error message in:
			 *   s -> IF LP expr RP expr | IF LP expr RP s ELSE s
			 * The precedence of the first production is the same
			 * as RP. Making ELSE higher precedence forces
			 * resolution in favor of the shift.
			 */

  			/* Abbreviations used in nonterminal names:
  			 *
  			 * abs    == abstract
  			 * arg(s) == argument(s)
  			 * const  == constant
  			 * decl   == declarator
  			 * def    == definition
  			 * expr   == expression
  			 * ext    == external
  			 * opt    == optional
  			 * param  == parameter
  			 * struct == structure
  			 */
%type <num> args const_expr test

%type <p_sym>   ext_decl_list ext_decl def_list def decl_list decl
%type <p_sym>   var_decl funct_decl local_defs new_name name enumerator
%type <p_sym>	name_list var_list param_declaration abs_decl abstract_decl
%type <p_val>	expr binary non_comma_expr unary initializer initializer_list
%type <p_val>	or_expr and_expr or_list and_list

%type <p_link>	type specifiers opt_specifiers type_or_class type_specifier
%type <p_sdef>	opt_tag tag struct_specifier
%type <p_char>	string_const target

/*----------------------------------------------------------------------
 * Global and external variables. Initialization to zero for all globals is
 * assumed. Since occs -a and -p is used, these variables may not be private.
 * The ifdef assures that space is allocated only once (this, header, section
 * will be placed in both yyact.c and yyout.c, YYACTION is defined only in
 * yyact.c, however.
 */

%{
#ifdef YYACTION
%}

/*@A -------------------------------------------------------------          */





/*@A -------------------------------------------------------------          */








%{
int     Nest_lev;	 /* Current block-nesting level.		*/
%}






%{
int	Enum_val;	 /* Current enumeration constant value	*/
%}


/*@A -------------------------------------------------------------          */



%{
char	Vspace[16];
char	Tspace[16];	 /* The compiler doesn't know the stack-frame size
			  * when it creates a link() directive, so it outputs
			  * a link(VSPACE+TSPACE). Later on, it #defines VSPACE
			  * to the size of the local-variable space and TSPACE
			  * to the size of the temporary-variable space. Vspace
			  * holds the actual name of the VSPACE macro, and
			  * Tspace the TSPACE macro. (There's a different name
			  * for each subroutine.)
			  */

char	Funct_name[ NAME_MAX+1 ];	/* Name of the current function */
%}





%{
#define STR_MAX 512	  	/* Maximum size of a string constant.   */
char	Str_buf[STR_MAX]; 	/* Place to assemble string constants.  */
%}


/*@A -------------------------------------------------------------          */




				/* Stacks. The stack macros are all in	*/
				/* <tools/stack.h>, included earlier	*/
%{
#include <tools/stack.h>    	/* Stack macros. (see Appendix A)	 */

int stk_err( o )	/* declared as int to keep the compiler happy */
int o;
{
    yyerror( o ? "Loop/switch nesting too deep or logical expr. too complex.\n"
	       : "INTERNAL, label stack underflow.\n"  );
    exit( 1 );
    BCC( return 0 );	/* keep the compiler happy */
}
#undef  stack_err
#define stack_err(o)	 stk_err(o)

stack_dcl (S_andor, int, 32);	/* This stack wouldn't be necessary if I were */
				/* willing to put a structure onto the value  */
				/* stack--or_list and and_list must both      */
				/* return 2 attributes; this stack will hold  */
				/* one of them.				      */
%}

/*@A -------------------------------------------------------------          */




%{
/* These stacks are necessary because there's no syntactic connection break,
 * continue, case, default and the affected loop-control statement.
 */

stack_dcl (S_brk,       int,    32); /* number part of current break target  */
stack_dcl (S_brk_label, char *, 32); /* string part of current break target  */

stack_dcl (S_con,  	int,    32); /* number part of current continue targ. */
stack_dcl (S_con_label, char *, 32); /* string part of current continue targ. */
%}


/*@A -------------------------------------------------------------          */





%{
int	Case_label = 0;		  /* Label used to process case statements. */

stack_dcl (S_switch, stab *, 32); /* Switch table for current switch.	    */
%}


/*@A -------------------------------------------------------------          */


%{
#endif /* ifdef YYACTION */
%}


%%
program : ext_def_list { clean_up(); }
	;

ext_def_list
	: ext_def_list ext_def
	| /* epsilon */
	  {
		yydata(   "#include <tools/virtual.h>\n" );
		yydata(   "#define  T(x)\n" 		 );
		yydata(   "SEG(data)\n" 	 	 );
		yycode( "\nSEG(code)\n" 	 	 );
		yybss ( "\nSEG(bss)\n"  	 	 );
	  }
	;

opt_specifiers
	: CLASS TTYPE {   set_class_bit(  0, $2->etype ); /* Reset class.   */
			  set_class_bit( $1, $2->etype ); /* Add new class. */
			  $$ = $2->type ;
		      }
	| TTYPE	      {   set_class_bit(0, $1->etype);  /* Reset class bits.*/
			  $$ = $1->type ;
		      }
	| specifiers
	| /* empty */						%prec COMMA
	  	      {
			  $$	    = new_link();
			  $$->class = SPECIFIER;
			  $$->NOUN  = INT;
		      }
	;
specifiers
	: type_or_class
	| specifiers type_or_class { spec_cpy( $$, $2 );
				     discard_link_chain( $2 ); }
	;
type
	: type_specifier
	| type type_specifier	{  spec_cpy( $$, $2 );
				   discard_link_chain( $2 ); }
	;
type_or_class
	: type_specifier
	| CLASS 	   	{  $$ = new_class_spec( $1 );  }
	;
type_specifier
	: TYPE		   	{ $$ = new_type_spec( yytext );	 }

	| enum_specifier   	{ $$ = new_type_spec( "int"  );	 }
	| struct_specifier 	{ $$ = new_link();
			     	  $$->class    = SPECIFIER;
				  $$->NOUN     = STRUCTURE;
				  $$->V_STRUCT = $1;
				}
	;


var_decl
	: new_name	     %prec COMMA  /* This production is done first. */

	| var_decl LP RP	    {	add_declarator( $$, FUNCTION ); }
	| var_decl LP var_list RP   {   add_declarator( $$, FUNCTION );
					discard_symbol_chain( $3 );
				    }
	| var_decl LB RB
	  {
		/* At the global level, this must be treated as an array of
		 * indeterminate size; at the local level this is equivalent to
		 * a pointer. The latter case is patched after the declaration
		 * is assembled.
		 */

		add_declarator( $$, ARRAY );
		$$->etype->NUM_ELE = 0;

		YYD( yycomment("Add POINTER specifier\n"); )
	  }

	| var_decl LB const_expr RB
	  {
		add_declarator( $$, ARRAY );
		$$->etype->NUM_ELE = $3;

		YYD(yycomment("Add array[%d] spec.\n", $$->etype->NUM_ELE);)
	  }
	| STAR var_decl			%prec UNOP
	  {
		add_declarator( $$ = $2, POINTER );
		YYD( yycomment("Add POINTER specifier\n"); )
	  }

	| LP var_decl RP { $$ = $2; }
	;

/*----------------------------------------------------------------------
 * Name productions. new_name always creates a new symbol, initialized with the
 * current lexeme. Name returns a preexisting symbol with the associated name
 * (if there is one); otherwise, the symbol is allocated. The NAME token itself
 * has a NULL attribute if the symbol doesn't exist, otherwise it returns a
 * pointer to a "symbol" for the name.
 */

new_name: NAME	{  $$ = new_symbol( yytext, Nest_lev );   }
	;

name	: NAME	{  if( !$1 || $1->level != Nest_lev )
		       $$ = new_symbol( yytext, Nest_lev );
		}
	;

/*----------------------------------------------------------------------
 * Global declarations: take care of the declarator part of the declaration.
 * (The specifiers are handled by specifiers).
 * Assemble the declarators into a chain, using the cross links.
 */

ext_decl_list
	:  ext_decl
	   {
		$$->next = NULL;		/* First link in chain. */
	   }
	|  ext_decl_list COMMA ext_decl
	   {
		/* Initially, $1 and $$ point at the head of the chain.
		 * $3 is a pointer to the new declarator.
		 */

		$3->next = $1;
		$$       = $3;
	   }
	;

ext_decl
	: var_decl
	| var_decl EQUAL initializer { $$->args = (symbol *)$3; }
	| funct_decl
	;



funct_decl
	: STAR funct_decl	      {   add_declarator( $$ = $2 , POINTER ); }
	| funct_decl LB RB	      {   add_declarator( $$, ARRAY );
					  $$->etype->NUM_ELE = 0;
				      }
	| funct_decl LB const_expr RB {   add_declarator( $$, ARRAY );
					  $$->etype->NUM_ELE = $3;
				      }
	| LP funct_decl RP	      {   $$ = $2; 			  }
	| funct_decl LP RP	      {   add_declarator( $$, FUNCTION ); }
	| new_name LP RP	      {   add_declarator( $$, FUNCTION ); }

	| new_name LP { ++Nest_lev; } name_list { --Nest_lev; } RP
	  {
		add_declarator( $$, FUNCTION );

		$4       = reverse_links( $4 );
		$$->args = $4;
	  }
	| new_name LP { ++Nest_lev; } var_list { --Nest_lev; } RP
	  {
		add_declarator( $$, FUNCTION );
		$$->args = $4;
	  }
	;
name_list
	: new_name		   {
					$$->next	 = NULL;
					$$->type	 = new_link();
					$$->type->class  = SPECIFIER;
					$$->type->SCLASS = AUTO;
				   }
	| name_list COMMA new_name {
					$$       	 = $3;
					$$->next 	 = $1;
					$$->type	 = new_link();
					$$->type->class  = SPECIFIER;
					$$->type->SCLASS = AUTO;
				    }
	;
var_list
	: param_declaration			{ if($1) $$->next = NULL; }
	| var_list COMMA param_declaration	{ if($3)
						  {
						      $$       = $3;
						      $3->next = $1;
						  }
						}
	;
param_declaration
	: type  var_decl    	{ add_spec_to_decl($1,  $$ = $2  ); }
	| abstract_decl		{ discard_symbol  ($1); $$ = NULL ; }
	| ELLIPSIS		{			$$ = NULL ; }
	;


abstract_decl
	: type 	abs_decl   {    add_spec_to_decl   ( $1, $$ = $2 ); }
	| TTYPE	abs_decl   {
				$$ = $2;
				add_spec_to_decl( $1->type, $2 );
			   }
	;

abs_decl
	: /* epsilon */		    { $$ = new_symbol("",0); 		   }
	| LP abs_decl RP LP RP	    { add_declarator( $$ = $2, FUNCTION ); }
	| STAR abs_decl		    { add_declarator( $$ = $2, POINTER  ); }
	| abs_decl LB            RB { add_declarator( $$,      POINTER  ); }
	| abs_decl LB const_expr RB { add_declarator( $$,      ARRAY 	);
				      $$->etype->NUM_ELE = $3;
				    }
	| LP abs_decl RP 	    { $$ = $2; }
	;

struct_specifier
	: STRUCT opt_tag LC def_list RC
	  {
		if( !$2->fields )
		{
		    $2->fields = reverse_links( $4 );

		    if( !illegal_struct_def( $2, $4 ) )
			$2->size = figure_struct_offsets( $2->fields, $1=='s' );
		}
		else
		{
		    yyerror("Ignoring redefinition of %s", $2->tag );
		    discard_symbol_chain( $4 );
		}

		$$ = $2;
	  }
	| STRUCT tag		 { $$ = $2; }
	;

opt_tag : tag
	| /* empty */	{
			    static unsigned label = 0;
			    static char     tag[16];
			    sprintf( tag, "%03d", label++ );

			    $$ = new_structdef( tag );
			    addsym( Struct_tab, $$ );
	  		}
	;

tag	: NAME		{
			    if( !($$=(structdef *) findsym(Struct_tab,yytext)))
			    {
				$$        = new_structdef( yytext );
				$$->level = Nest_lev;
				addsym( Struct_tab, $$ );
			    }
			}
	;


def_list
	: def_list def		{   symbol *p;
				    if( p = $2 )
				    {
					for(; p->next; p = p->next )
					    ;
					p->next = $1;
					$$      = $2;
				    }
				}
	| /* epsilon */		{   $$ = NULL; }   /* Initialize end-of-list */
	;					   /* pointer.		     */

def
	: specifiers decl_list	{ add_spec_to_decl( $1, $2 );	}
	  		  SEMI	{ $$ = $2;			}

	| specifiers SEMI 	{ $$ = NULL; }
	;

decl_list
	: decl 			{ $$->next = NULL;}
	| decl_list COMMA decl
	  {
		$3->next = $1;
		$$       = $3;
	  }
	;

decl
	: funct_decl
	| var_decl
	| var_decl EQUAL initializer {	yyerror( "Ignoring initializer.\n");
					discard_value( $3 );
				     }
	| var_decl COLON const_expr   		%prec COMMA
	| COLON const_expr		    	%prec COMMA
	;

enum_specifier
	: enum name opt_enum_list    {	if( $2->type )
					   yyerror("%s: redefinition",$2->name);
					else
					   discard_symbol($2);
				     }
	| enum LC enumerator_list RC
	;

opt_enum_list
	: LC enumerator_list RC
	| /* empty */
	;


enum	: ENUM   { Enum_val = 0; }

	;

enumerator_list
	: enumerator
	| enumerator_list COMMA enumerator
	;

enumerator
	: name			{   do_enum( $1, Enum_val++ );  }
	| name EQUAL const_expr {   Enum_val = $3;
				    do_enum( $1, Enum_val++ );  }
	;


compound_stmt
	: LC			   {   if( ++Nest_lev == 1 )
					   loc_reset();
				   }
	  local_defs stmt_list RC  {   --Nest_lev;
				       remove_symbols_from_table ( $3 );
				       discard_symbol_chain      ( $3 );
				   }
	;

local_defs
	: def_list	{   add_symbols_to_table ( $$ = reverse_links( $1 ));
			    figure_local_offsets ( $$, Funct_name	    );
			    create_static_locals ( $$, Funct_name	    );
			    print_offset_comment ( $$, "variable"	    );
			}
	;



ext_def : opt_specifiers ext_decl_list
	  {
		add_spec_to_decl( $1, $2 );

		if( !$1->tdef )
		    discard_link_chain( $1 );

		add_symbols_to_table 		( $2 = reverse_links( $2 ) );
		figure_osclass			( $2	 );
		generate_defs_and_free_args     ( $2	 );
		remove_duplicates		( $2     );
	  }
	  SEMI





	| opt_specifiers
	  {
		if( !($1->class == SPECIFIER && $1->NOUN == STRUCTURE) )
		    yyerror("Useless definition (no identifier)\n");
		if( !$1->tdef )
		    discard_link_chain( $1 );
	  }
	  SEMI


	| opt_specifiers funct_decl
	  {
		static unsigned link_val = 0;	 /* Labels used for link args.*/

		add_spec_to_decl( $1, $2 );	 /* Merge the specifier and   */
						 /*  		  declarator. */
		if( !$1->tdef )
		    discard_link_chain( $1 );    /* Discard extra specifier.  */

		figure_osclass	     ( $2 );	      /* Update symbol table. */
		add_symbols_to_table ( $2 ); 	      /* Add function itself. */
		add_symbols_to_table ( $2->args );    /* Add the arguments.   */

		strcpy ( Funct_name, $2->name		    );
		sprintf( Vspace, "%s%d", L_LINK, link_val++ );
		sprintf( Tspace, "%s%d", L_LINK, link_val++ );

		yycode( "\n#undef   T\n" );
		yycode(   "#define  T(n) (fp-(%s*%d)-(n*%d))\n\n",
						       Vspace, SWIDTH, SWIDTH );
		gen( "PROC", $2->rname, $2->etype->STATIC ? "private":"public");
		gen( "link", Vspace, Tspace );

		++Nest_lev;	  /* Make nesting level of definition_list
				   * match nesting level in the funct_decl
				   */
	  }
	  def_list
	  {
		fix_types_and_discard_syms ( $4			  );
		figure_param_offsets	   ( $2->args		  );
		print_offset_comment	   ( $2->args, "argument" );

		--Nest_lev;    /* It's incremented again in the compound_stmt */
	  }
	  compound_stmt
	  {
		purge_undecl();		   /* Deal with implicit declarations */
					   /* and undeclared symbols.	      */

		remove_symbols_from_table ( $2->args );  /* Delete arguments. */
		discard_symbol_chain      ( $2->args );

		gen( ":%s%d", L_RET, rlabel(1) );     /* End-of-function */
		gen( "unlink" 		       );     /*           code. */
		gen( "ret"    		       );
		gen( "ENDP",  $2->rname	       );

		yybss ( "\n#define  %s %d\t/* %s: locals */\n",
					Vspace, loc_var_space(), $2->name );
		yybss (   "#define  %s %d\t/* %s: temps. */\n",
					Tspace, tmp_var_space(), $2->name );

		tmp_reset();		   /* Reset temporary-variable system.*/
					   /* (This is just insurance.)       */
	  }
	;



stmt_list
	: stmt_list statement
	| /* epsilon */
	;

/*----------------------------------------------------------------------
 * Statements
 */

statement
	: SEMI
	| compound_stmt
	| expr SEMI  		{ release_value($1); tmp_freeall(); }
	| RETURN 	SEMI 	{ gen( "goto%s%d", L_RET, rlabel(0) ); }

	| RETURN expr	SEMI    { gen("=", IS_INT    ($2->type) ? "rF.w.low" :
					   IS_POINTER($2->type) ? "rF.pp"    :
								  "rF.l",
								   rvalue($2) );
				  gen( "goto%s%d", L_RET, rlabel(0) );
				  release_value( $2 );
				  tmp_freeall();
				}
	| GOTO target SEMI	{  gen("goto",$2); }
	| target COLON		{  gen(":",   $1); }
	 	      statement

	| IF LP test RP statement	{    gen( ":%s%d", L_NEXT, $3 );
					}

	| IF LP test RP statement ELSE  {   gen( "goto%s%d", L_ELSE, $3 );
					    gen( ":%s%d",    L_NEXT, $3 );
					}
	  statement			{   gen( ":%s%d",    L_ELSE, $3 );
					}






	| WHILE LP test RP	{   push(S_con, $3); push( S_con_label, L_TEST);
				    push(S_brk, $3); push( S_brk_label, L_NEXT);
			      	}
	  statement		{   gen( "goto%s%d", L_TEST, $3 );
				    gen( ":%s%d",    L_NEXT, $3 );
				    pop( S_con ); pop( S_con_label );
				    pop( S_brk ); pop( S_brk_label );
			        }

	| DO			{   static int label;

				    gen(":%s%d", L_DOTOP, $<num>$ = ++label );
				    push( S_con,	label );
				    push( S_con_label,	L_DOTEST );
				    push( S_brk,	label );
				    push( S_brk_label,	L_DOEXIT  );
			        }
	  statement WHILE	{   gen(":%s%d",    L_DOTEST,  $<num>2 ); }
	  LP test RP SEMI	{   gen("goto%s%d", L_DOTOP,   $<num>2 );
				    gen( ":%s%d",   L_DOEXIT,  $<num>2 );
				    gen( ":%s%d",   L_NEXT,    $7 	);
				    pop( S_con );
				    pop( S_con_label );
				    pop( S_brk );
				    pop( S_brk_label );
			        }

	| FOR LP opt_expr SEMI
		 test     SEMI  {
				    gen("goto%s%d", L_BODY,      $5 );
				    gen(":%s%d",    L_INCREMENT, $5 );

				    push(S_con,	$5);
				    push(S_con_label,	L_INCREMENT );
				    push(S_brk,	$5);
				    push(S_brk_label,	L_NEXT	    );
			        }
	  opt_expr RP	        {   gen("goto%s%d", L_TEST,	 $5 );
				    gen(":%s%d",    L_BODY,      $5 );
			        }
	  statement	        {   gen("goto%s%d", L_INCREMENT, $5 );
				    gen( ":%s%d",   L_NEXT,	 $5 );

				    pop( S_con 	     );
				    pop( S_con_label );
				    pop( S_brk       );
				    pop( S_brk_label );
			        }

	| BREAK SEMI		{   if( stack_empty(S_brk) )
					yyerror("Nothing to break from\n");

				    gen_comment("break");
				    gen("goto%s%d", stack_item(S_brk_label,0),
						     stack_item(S_brk,      0));
				}

	| CONTINUE SEMI		{   if( stack_empty(S_brk) )
					yyerror("Continue not in loop\n");

				    gen_comment("continue");
				    gen("goto%s%d", stack_item(S_con_label,0),
						     stack_item(S_con,0      ));
			        }



	| SWITCH LP expr RP
	  {
	       /* Note that the end-of-switch label is the 2nd argument to
		* new_stab + 1; This label should be used for breaks when in
		* the switch.
		*/

		push( S_switch,	new_stab($3, ++Case_label) );
		gen_comment("Jump to case-processing code" );
		gen("goto%s%d", L_SWITCH, Case_label );

		push( S_brk,	   ++Case_label	);
		push( S_brk_label, L_SWITCH   	);

		release_value( $3 );
		tmp_freeall();
	  }
	  compound_stmt
	  {
		gen_stab_and_free_table( pop( S_switch ) );
		pop( S_brk );
		pop( S_brk_label );
	  }

	| CASE const_expr COLON
	  {
		add_case    ( stack_item(S_switch,0), $2, ++Case_label );
		gen_comment ( "case %d:", $2 		 	       );
		gen	    ( ":%s%d"  , L_SWITCH, Case_label	       );
	  }

	| DEFAULT COLON
	  {
		add_default_case( stack_item(S_switch,0), ++Case_label );
		gen_comment("default:");
		gen(":%s%d", L_SWITCH, Case_label );
	  }
	;

target	: NAME	      		{   static char buf[ NAME_MAX ];
				    sprintf(buf, "_%0.*s", NAME_MAX-2, yytext );
				    $$ = buf;
			        }
	;

test	:	      		{   static int label = 0;
				    gen( ":%s%d", L_TEST, $<num>$ = ++label );
			        }
	  expr        	        {
				    $$ = $<num>1;
				    if( IS_INT_CONSTANT($2->type) )
				    {
					if( ! $2->type->V_INT )
					    yyerror("Test is always false\n");
				    }
				    else  /* not an endless loop */
				    {
					gen( "EQ",	     rvalue($2), "0" );
					gen( "goto%s%d", L_NEXT,     $$  );
				    }
				    release_value( $2 );
				    tmp_freeall();
				}
	| /* empty */		{   $$ = 0;  /* no test */
				}
	;
unary
	: LP expr RP 	{ $$ = $2;		      			}
	| FCON	  	{ yyerror("Floating-point not supported\n");	}
	| ICON	  	{ $$ = make_icon ( yytext,  0 );		}
	| NAME 		{ $$ = do_name   ( yytext, $1 );		}



	| string_const 	%prec COMMA
			{
			  $$ = make_scon();
			  yydata( "private\tchar\t%s[]=\"%s\";\n", $$->name,$1);
			}




	| SIZEOF LP string_const RP				%prec SIZEOF
				     { $$ = make_icon(NULL,strlen($3) + 1 ); }

	| SIZEOF LP expr RP	     				%prec SIZEOF
				     { $$ = make_icon(NULL,get_sizeof($3->type));
				       release_value( $3 );
				     }
	| SIZEOF LP abstract_decl RP				%prec SIZEOF
				     {
					 $$ = make_icon( NULL,
							  get_sizeof($3->type));
					 discard_symbol( $3 );
				     }



	| LP abstract_decl RP unary    				  %prec UNOP
	  {
		if( IS_AGGREGATE($2->type) )
		{
		    yyerror( "Illegal cast to aggregate type\n" );
		    $$ = $4;
		}
		else if( IS_POINTER($2->type) && IS_POINTER($4->type) )
		{
		    discard_link_chain( $4->type );
		    $4->type  = $2->type;
		    $4->etype = $2->type;
		    $$ = $4;
		}
		else
		{
		    $$ = tmp_gen( $2->type, $4 );
		}
	  }



	| MINUS	  unary		{ $$ = do_unop( '-', $2 );   }   %prec UNOP
	| UNOP	  unary		{ $$ = do_unop( $1,  $2 );   }




	| unary INCOP		{ $$ = incop( 0, $2, $1 );    }
	| INCOP	unary		{ $$ = incop( 1, $1, $2 );    }




	| AND	  unary 	{ $$ = addr_of ( $2	 );   }   %prec UNOP
	| STAR unary		{ $$ = indirect( NULL, $2 );  }	  %prec UNOP
	| unary LB expr RB	{ $$ = indirect( $3,   $1 );  }	  %prec UNOP
	| unary STRUCTOP NAME { $$ = do_struct($1, $2, yytext); } %prec STRUCTOP



	| unary LP args RP  	{ $$ = call    ( $1,  $3 );   }
	| unary LP      RP  	{ $$ = call    ( $1,  0  );   }
	;

args    : non_comma_expr  %prec COMMA   {   gen( "push", rvalue( $1 ) );
					    release_value( $1 );
					    $$ = 1;
				        }
	| non_comma_expr COMMA args	{   gen( "push", rvalue( $1 ) );
					    release_value( $1 );
					    $$ = $3 + 1;
					}
	;


expr	: expr COMMA  {release_value($1);}  non_comma_expr  { $$=$4; }
	| non_comma_expr
	;

non_comma_expr
	: non_comma_expr QUEST	{   static int label = 0;

				    if( !IS_INT($1->type) )
				       yyerror("Test in ?: must be integral\n");

				    gen( "EQ",	     rvalue( $1 ), "0" );
				    gen( "goto%s%d", L_COND_FALSE,
							    $<num>$ = ++label );
				    release_value( $1 );
			        }
	  non_comma_expr COLON  {   $<p_val>$ = $4->is_tmp
						    ? $4
						    : tmp_gen($4->type, $4)
						    ;

				    gen( "goto%s%d", L_COND_END,   $<num>3 );
				    gen( ":%s%d",    L_COND_FALSE, $<num>3 );
				}
	  non_comma_expr	{   $$ = $<p_val>6;

				    if( !the_same_type($$->type, $7->type, 1) )
					yyerror(
					"Types on two sides of : must agree\n");

				    gen( "=",     $$->name,   rvalue($7) );
				    gen( ":%s%d", L_COND_END, $<num>3    );
				    release_value( $7 );
			        }

	| non_comma_expr ASSIGNOP non_comma_expr {$$ = assignment($2, $1, $3);}
	| non_comma_expr EQUAL    non_comma_expr {$$ = assignment( 0, $1, $3);}

	| or_expr
	;




or_expr : or_list         {   int label;
			      if( label = pop( S_andor ) )
				  $$ = gen_false_true( label, NULL );
		          }
	;
or_list : or_list OROR    {   if( $1 )
				  or( $1, stack_item(S_andor,0) = tf_label());
		          }
	  and_expr        {   or( $4, stack_item(S_andor,0) );
			      $$ = NULL;
		          }
	| and_expr        {   push( S_andor, 0 );
			  }
	;



and_expr: and_list        {	int label;
				if( label = pop( S_andor ) )
				{
				    gen( "goto%s%d", L_TRUE, label );
				    $$ = gen_false_true( label, NULL );
				}
		          }
	;
and_list: and_list ANDAND {	if( $1 )
				    and($1, stack_item(S_andor,0) = tf_label());
		          }
	  binary          {	and( $4, stack_item(S_andor,0) );
				    $$ = NULL;
			  }
	| binary          {	push( S_andor, 0 );
			  }
	;


binary
	: binary RELOP	  binary	{ $$ = relop( $1, $2,  $3 ); }
	| binary EQUOP	  binary	{ $$ = relop( $1, $2,  $3 ); }




	| binary STAR 	  binary	{ $$ = binary_op( $1, '*', $3 ); }
	| binary DIVOP 	  binary	{ $$ = binary_op( $1, $2,  $3 ); }
	| binary SHIFTOP  binary	{ $$ = binary_op( $1, $2,  $3 ); }
	| binary AND	  binary	{ $$ = binary_op( $1, '&', $3 ); }
	| binary XOR	  binary	{ $$ = binary_op( $1, '^', $3 ); }
	| binary OR	  binary	{ $$ = binary_op( $1, '|', $3 ); }




	| binary PLUS 	  binary	{ $$ = plus_minus( $1, '+', $3 ); }
	| binary MINUS 	  binary	{ $$ = plus_minus( $1, '-', $3 ); }
	| unary
	;



opt_expr
	: expr 		  { release_value( $1 ); tmp_freeall(); }
	| /* epsilon */
	;

const_expr
	: expr				%prec COMMA
			  {
				$$ = -1 ;

				if( !IS_CONSTANT( $1->type ) )
				    yyerror("Constant required.");

				else if( !IS_INT( $1->type ) )
				    yyerror("Constant expression must be int.");

				else
				    $$ = $1->type->V_INT ;

				release_value($1);
				tmp_freeall();
			  }
	;

initializer : expr					%prec COMMA
	    | LC initializer_list RC	{ $$ = $2; }
	    ;

initializer_list
 	    : initializer
	    | initializer_list COMMA initializer
	      {
		    yyerror("Aggregate initializers are not supported\n");
		    release_value( $3 );
	      }
	    ;


string_const
	: STRING
	  {
		$$	 = Str_buf;
		*Str_buf = '\0';

		yytext[ strlen(yytext) - 1 ] = '\0' ;	/* Remove trailing " */

		if( concat(STR_MAX, Str_buf, Str_buf, yytext+1, NULL) < 0 )
		    yyerror("String truncated at %d characters\n", STR_MAX );
	  }
	| string_const STRING
	  {
		yytext[ strlen(yytext) - 1 ] = '\0' ;	/* Remove trailing " */

		if( concat(STR_MAX, Str_buf, Str_buf, yytext+1, NULL) < 0 )
		    yyerror("String truncated at %d characters\n", STR_MAX );
	  }
	;

%%
#define OFILE_NAME "output.c"	/* Output file name.		*/
char	*Bss ;		 	/* Name of BSS  temporary file.	*/
char	*Code;		 	/* Name of Code temporary file.	*/
char	*Data;		 	/* Name of Data temporary file.	*/

static  void init_output_streams P((char **p_code,char **p_data, char **p_bss));
static  void sigint_handler	 P((void				     ));
/*----------------------------------------------------------------------*/

PRIVATE void init_output_streams( p_code, p_data, p_bss)
char	**p_code, **p_data, **p_bss;
{
    /* Initialize the output streams, making temporary files as necessary.
     * Note that the ANSI tmpfile() or the UNIX mkstmp() functions are both
     * better choices than the mktemp()/fopen() used here because another
     * process could, at least in theory, sneak in between the two calls.
     * Since mktemp uses the process id as part of the file name, this
     * is not much of a problem, and the current method is more portable
     * than tmpfile() or mkstmp(). Be careful in a network environment.
     */

    if( !(*p_code = mktemp("ccXXXXXX")) || !(*p_data = mktemp("cdXXXXXX")) ||
	!(*p_bss  = mktemp("cbXXXXXX"))
      )
    {
	yyerror("Can't create temporary-file names");
	exit( 1 );
    }

    if( !(yycodeout=fopen(*p_code, "w")) || !(yydataout=fopen(*p_data, "w")) ||
	!(yybssout =fopen( *p_bss, "w"))
      )
    {
	perror("Can't open temporary files");
	exit( 1 );
    }
}

/*----------------------------------------------------------------------*/
#ifdef __TURBOC__
  void _Cdecl (* _Cdecl Osig) (int);
#else
void	(*Osig) P((int));  /* Previous SIGINT handler.Initialized in 	*/
			   /*				yy_init_occs().	*/
#endif

PRIVATE void sigint_handler()
{
    /* Ctrl-C handler. Note that the debugger raises SIGINT on a 'q' command,
     * so this routine is executed when you exit the debugger. Also, the
     * debugger's own SIGINT handler, which cleans up windows and so forth, is
     * installed before yy_init_occs() is called. It's called here to clean up
     * the environment if necessary. If the debugger isn't installed, the call
     * is harmless.
     */

    signal   ( SIGINT, SIG_IGN	);
    clean_up (			);
    unlink   ( OFILE_NAME	);
    (*Osig)(0);
    exit(1);			/* Needed only if old signal handler returns. */
}
/*----------------------------------------------------------------------*/
sym_cmp    (s1, s2) symbol    *s1, *s2; { return strcmp  (s1->name, s2->name);}
struct_cmp (s1, s2) structdef *s1, *s2; { return strcmp  (s1->tag,  s2->tag );}

unsigned sym_hash     (s1)   symbol    *s1;  { return hash_pjw(s1->name ); }
unsigned struct_hash  (s1)   structdef *s1;  { return hash_pjw(s1->tag  ); }

PUBLIC  void	yy_init_occs( val )
void *val;				/* void* to match function prototype. */
{					/* Exact match reqired by Borland C.  */
    Osig = signal( SIGINT, SIG_IGN );
    init_output_streams( &Code, &Data, &Bss );
    signal( SIGINT, BCC( (void _Cdecl  (*)(   )) )
		    MSC( (void (__cdecl *)(int)) ) sigint_handler );

    ((yystype *)val)->p_char = "---";	/* Attribute for the start symbol. */

    Symbol_tab = maketab( 257, sym_hash,    sym_cmp    );
    Struct_tab = maketab( 127, struct_hash, struct_cmp );
}

/*----------------------------------------------------------------------*/

PRIVATE void clean_up()
{
    /* Cleanup actions. Mark the ends of the various segments, then merge the
     * three temporary files used for the code, data, and bss segments into a
     * single file called output.c. Delete the temporaries. Since some compilers
     * don't delete an existing file with a rename(), it's best to assume the
     * worst. It can't hurt to delete a nonexistent file, you'll just get an
     * error back from the operating system.
     */

    extern FILE *yycodeout, *yydataout, *yybssout;

    signal ( SIGINT, SIG_IGN );
    fclose ( yycodeout	     );
    fclose ( yydataout	     );
    fclose ( yybssout	     );
    unlink ( OFILE_NAME	     );	/* delete old output file (ignore EEXIST) */

    if( rename( Data, OFILE_NAME ) )
	yyerror("Can't rename temporary (%s) to %s\n", Data, OFILE_NAME );
    else
    {					      /* Append the other temporary   */
	movefile( OFILE_NAME, Bss , "a" );    /* files to the end of the      */
	movefile( OFILE_NAME, Code, "a" );    /* output file and delete the   */
    }					      /* temporary files. movefile()  */
}					      /* is in appendix A.	      */
enum union_fields { NONE, P_SYM, P_LINK,   P_SDEF,    P_FIELD, P_SPEC,
			  P_CHAR, P_VALUE, SYM_CHAIN, ASCII, NUM 	};
typedef struct tabtype
{
    char *sym;
    enum union_fields case_val;
} tabtype;


static int tcmp P(( tabtype *p1, tabtype *p2 ));	/* declared below */

tabtype Tab[] =
{
	/*    nonterminal		 field		*/
	/*	name	      		in %union	*/

	{ "ASSIGNOP",			ASCII		},
	{ "CLASS",			ASCII 		},
	{ "DIVOP",			ASCII		},
	{ "EQUOP",			ASCII		},
	{ "INCOP",			ASCII		},
	{ "NAME",			P_SYM 		},
	{ "RELOP",			ASCII		},
	{ "SHIFTOP",			ASCII		},
	{ "STRUCT",			ASCII		},
	{ "STRUCTOP",			ASCII		},
	{ "TTYPE",			P_SYM 		},
	{ "UNOP",			ASCII		},
	{ "abs_decl",			P_SYM		},
	{ "abstract_decl",		P_SYM		},
	{ "and_expr",			P_VALUE		},
	{ "and_list",			P_VALUE		},
	{ "args",			NUM		},
	{ "binary",			P_VALUE		},
	{ "const_expr",			NUM		},
	{ "decl",			P_SYM		},
	{ "decl_list",			SYM_CHAIN 	},
	{ "def",			SYM_CHAIN 	},
	{ "def_list",			SYM_CHAIN	},
	{ "enumerator",			P_SYM 		},
	{ "expr",			P_VALUE		},
	{ "ext_decl",			P_SYM 		},
	{ "ext_decl_list",		SYM_CHAIN 	},
	{ "funct_decl",			P_SYM 		},
	{ "initializer",		P_VALUE		},
	{ "initializer_list",		P_VALUE		},
	{ "local_defs",			SYM_CHAIN	},
	{ "name",			P_SYM 		},
	{ "name_list",			SYM_CHAIN	},
	{ "new_name",			P_SYM 		},
	{ "non_comma_expr",		P_VALUE		},
	{ "opt_specifiers",		P_LINK 		},
	{ "opt_tag",			P_SDEF 		},
	{ "or_expr",			P_VALUE		},
	{ "or_list",			P_VALUE		},
	{ "param_declaration",		SYM_CHAIN	},
	{ "specifiers",			P_LINK 		},
	{ "string_const",		P_CHAR 		},
	{ "struct_specifier",		P_SDEF 		},
	{ "tag",			P_SDEF 		},
	{ "test",			NUM 		},
	{ "type",			P_LINK 		},
	{ "type_or_class",		P_LINK 		},
	{ "type_specifier",		P_LINK 		},
	{ "unary",			P_VALUE		},
	{ "var_decl",			P_SYM 		},
	{ "var_list",			SYM_CHAIN	},
	{ "{72}",			NUM		},
	{ "{73}",			P_VALUE		}
};

static int tcmp( p1, p2 )
tabtype *p1, *p2;
{
    return( strcmp(p1->sym, p2->sym) );
}

char 	*yypstk( val, name )
void	*val;		/* Ptr. to value-stack item. void* to match prototype */
char	*name;		/* Ptr. to debug-stack item. */
{
    static char buf[128];
    char	*text;
    tabtype	*tp, template;
    yystype	*v = (yystype *)val;	/* avoid casts all over the place */

    template.sym = name;

    tp = (tabtype *) bsearch( &template, Tab, sizeof(Tab)/sizeof(*Tab),
  							    sizeof(*Tab),
	 BCC( (int _Cdecl(  *)(const void _FAR *, const void _FAR *)) )
	 MSC( (int (__cdecl *)(const void      *, const void      *)) ) tcmp);

    sprintf( buf, "%04x ", v->num );	/* The first four characters in the */
    text = buf + 5;			/* string are the numeric value of  */
					/* the current stack element.       */
					/* Other text is written at "text". */
    switch( tp ? tp->case_val : NONE )
    {
    case SYM_CHAIN:
		 sprintf( text, "sym chain: %s",
				v->p_sym ? sym_chain_str(v->p_sym) : "NULL" );
		 break;
    case P_SYM:
		 if( ! v->p_sym )
		     sprintf( text, "symbol: NULL" );

		 else if( IS_FUNCT(v->p_sym->type) )
		     sprintf( text, "symbol: %s(%s)=%s %1.40s",
				v->p_sym->name,
				sym_chain_str( v->p_sym->args ),
				v->p_sym->type && *(v->p_sym->rname) ?
							v->p_sym->rname : "",
				type_str(v->p_sym->type) );
		 else
		     sprintf( text, "symbol: %s=%s %1.40s",
				v->p_sym->name,
				v->p_sym->type && *(v->p_sym->rname) ?
							v->p_sym->rname : "",
				type_str(v->p_sym->type) );
		 break;
    case P_SPEC:
		 if( !v->p_spec )
		     sprintf( text, "specifier: NULL" );
		 else
		     sprintf( text, "specifier: %s %s", attr_str( v->p_spec ),
						  noun_str( v->p_spec->noun ) );
		 break;
    case P_LINK:
		 if( !v->p_link )
		     sprintf( text, "specifier: NULL" );
		 else
		     sprintf( text, "link: %1.50s", type_str(v->p_link) );
		 break;
    case P_VALUE:
		 if( !v->p_val )
		     sprintf( text, "_value: NULL" );
		 else
		 {
		     sprintf( text, "%cvalue: %s %c/%u %1.40s",
				    v->p_val->lvalue   ? 'l'	        : 'r' ,
				    *(v->p_val->name)  ? v->p_val->name : "--",
				    v->p_val->is_tmp   ? 't'	        : 'v' ,
				    v->p_val->offset,
				    type_str( v->p_val->type ) );
		 }
		 break;
    case P_SDEF:
		 if( !v->p_sdef )
		     sprintf( text, "structdef: NULL" );
		 else
		     sprintf( text, "structdef: %s lev %d, size %d",
							       v->p_sdef->tag,
							       v->p_sdef->level,
							       v->p_sdef->size);
		 break;
    case P_CHAR:
		 if( !v->p_sdef )
		     sprintf( text, "string: NULL" );
		 else
		     sprintf( text, "<%s>", v->p_char );
		 break;
    case NUM:
		 sprintf( text, "num: %d", v->num );
		 break;
    case ASCII:
		 sprintf( text, "ascii: `%s`", bin_to_ascii(v->ascii, 1) );
		 break;
    }
    return buf;
}
