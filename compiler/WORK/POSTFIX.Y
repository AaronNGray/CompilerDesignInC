/*@A (C) 1992 Allen I. Holub                                                */

%term ICON  NAME
%left PLUS  MINUS
%left TIMES DIVIDE
%%
s	: expr
	;

expr	: NAME			{ yycode("%s\n", yytext	); }
	| ICON			{ yycode("%s\n", yytext	); }
	| expr DIVIDE expr	{ yycode("/\n" 		); }
	| expr TIMES  expr	{ yycode("*\n" 		); }
	| expr PLUS   expr	{ yycode("+\n" 		); }
	| expr MINUS  expr	{ yycode("-\n" 		); }
	;
%%
main(){ yyparse(); }
