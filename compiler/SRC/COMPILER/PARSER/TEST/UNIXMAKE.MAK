#@A (C) 1992 Allen I. Holub 
# This makefile puts together the expression compiler. The Microsoft C
# compiler is used, and I've assumed that you've added whatever directory
# holds l.lib curses.lib comp.lib and termlib.lib to your LIB environment.
#
# Use "make yyexpr" to make the occs  version.
# Use "make llexpr" to make the llama version.
#

LIB   = /violet_b/holub/compiler/lib/comp.lib \
			/violet_b/holub/compiler/lib/l.lib -lcurses -ltermlib
OCCS  = /violet_b/holub/compiler/bin/occs
LLAMA = /violet_b/holub/compiler/bin/llama
LEX   = /violet_b/holub/compiler/bin/LeX

CFLAGS = -I/violet_b/holub/compiler/include


yyexpr:		yyout.o lexyy.o
		$(CC) $(CFLAGS) -o yyexpr yyout.o lexyy.o $(LIB)

yyout.o:	yyout.c
yyout.c:	expr.y
		$(OCCS) -vDl expr.y

lexyy.o:	lexyy.c
lexyy.c:	expr.lex
		$(LEX) -vl expr.lex

llexpr:		llout.o llexyy.o
		$(CC) -o llexpr llout.o llexyy.o $(LIB)

llexyy.o:	llexyy.c
llexyy.c:	llexpr.lex
		$(LEX) -vl llexpr.lex
		mv lexyy.c llexyy.c

llout.o:	llout.c
llout.c:	expr.lma
		$(LLAMA) -vDl expr.lma
