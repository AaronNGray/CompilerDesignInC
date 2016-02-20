/*@A (C) 1992 Allen I. Holub                                                */


%{
#include "yyout.h"
%}
digit	[0-9]
alpha	[a-zA-Z_]
alnum	[0-9a-zA-Z_]
%%

"+"		return PLUS;
"*"		return STAR;
"("		return LP;
")"		return RP;
{digit}+	|
{alpha}{alnum}*	return NUM_OR_ID;
.		;
%%
