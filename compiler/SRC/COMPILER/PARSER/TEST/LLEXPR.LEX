/*@A (C) 1992 Allen I. Holub                                                */
%{
#include "llout.h"
%}
%%

"+"	return PLUS;
"*"	return TIMES;
"("	return LP;
")"	return RP;
";"	return SEMI;
[0-9]+	|
[a-z_]+	return NUM_OR_ID;
%%
