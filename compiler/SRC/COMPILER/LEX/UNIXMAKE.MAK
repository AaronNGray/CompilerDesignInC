#@A (C) 1992 Allen I. Holub 
OBJ	= lex.o squash.o minimize.o dfa.o input.o nfa.o \
	  print.o printnfa.o signon.o terp.o

INCLUDE	= /violet_b/holub/compiler/include
CFLAGS  = -I$(INCLUDE) -g

#--------------------------------------------------------------
# the -lcl is gets the Berkeley library that contains getcwd(), which is used
# in searchen.c in comp.lib (source in .../src/compiler/lib.) You may have to
# change this switch if your getcwd() is in a different library [or modify
# searchen.c if you don't have a getcwd()].

LeX:  $(OBJ)
	cc -o LeX $(OBJ) ../../../lib/comp.lib -lcl
	rm date.h

#--------------------------------------------------------------

lexyy.o: lexyy.c
lexyy.c:   test.lex LeX
	LeX -v test.lex

#--------------------------------------------------------------

input.o:	input.c			      globals.h
lex.o:		lex.c		dfa.h  nfa.h  globals.h
minimize.o:	minimize.c 	dfa.h 	      globals.h
nfa.o:		nfa.c		       nfa.h  globals.h
print.o:	print.c		dfa.h  nfa.h  globals.h
printnfa.o:	printnfa.c 	       nfa.h
signon.o:	signon.c				  date.h
squash.o:	squash.c 	dfa.h 	      globals.h
dfa.o:		dfa.c		dfa.h 	      globals.h
terp.o:		terp.c		nfa.h 	      globals.h

date.h:
	echo \#define __DATE__ \"`date`\" >> date.h

#----------------------------------------------------------------------
# test LeX by processing the C-compiler's lexical analyzer

test:
	LeX -v -m../../../lib/lex.par ../c/c.lex
