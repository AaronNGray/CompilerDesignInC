/*@A (C) 1992 Allen I. Holub                                                */
%{
#include "yyout.h"
%}
%%
"+"	return PLUS;
"*"	return STAR;
"("	return LP;
")"	return RP;
[0-9]+	return NUM;
[a-z]+  return ID;
%%
