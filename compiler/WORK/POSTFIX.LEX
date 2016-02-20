/*@A (C) 1992 Allen I. Holub                                                */
%{
#include "yyout.h"
%}
%%
[a-zA-Z_][a-zA-Z_0-9]*	return NAME;
[0-9]+			return ICON;
"/"			return DIVIDE;
"*"			return TIMES;
"+"			return PLUS;
"-"			return MINUS;

.			; /* empty */
%%
