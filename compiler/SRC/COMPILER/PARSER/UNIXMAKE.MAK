#@A (C) 1992 Allen I. Holub 

# UNIX version of the makefile. You should make llama first, starting out
# with the llpar.c version, then once that works, recompiling in the llout.c
# version. You do this by modifying the LLPAR macro definition, below. Once
# llama works, make occs by redefining PROG macro and "target" dependancy,
# below.
#----------------------------------------------------------------------
# LEX     is the full path name of LeX.
# INCLUDE is the root directory of the compiler-specific include files
# CLFAGS  are the arguments to the c compiler (cc by default)
# PP	  is the full path name of the token-pasting shell script from the
#	  distribution disk.

LEX	 = /violet_b/holub/compiler/bin/LeX
LLAMA	 = llama
INCLUDE  = /violet_b/holub/compiler/include
PP	 = /violet_b/holub/compiler/bin/pp
CFLAGS   = $(PROG) -I$(INCLUDE) -g

#----------------------------------------------------------------------
# CLIB is a list of the libraries needed by llama and occs. The -lcl is gets the
# Berkeley library that contains getcwd(), which is used in searchen.c in
# comp.lib (source in .../src/compiler/lib.) You may have to change this switch
# if your getcwd() is in a different library [or modify searchen.c if you don't
# have a getcwd()].

CLIB	 = /violet_b/holub/compiler/lib/comp.lib \
	   /violet_b/holub/compiler/lib/l.lib -lcl

#----------------------------------------------------------------------
# Comment out the first two directives to compile occs, the second two to
# compile llama.
#				Compile OCCS
PROG 	 = -DOCCS
target: occs
#
#				Compile LLAMA,
# PROG	= -DLLAMA
# target: llama

#----------------------------------------------------------------------
# Use llpar.o for the recursive-descent parser in llama -- use llout.o for
# the llama-generated parser. The latter is always used for occs, reguardless of
# the value of LLPAR. If you're starting from scratch (without a working llama),
# you whould compiler at first with LLPAR set to llpar.o. Once that version of
# llama is working. Redefine LLPAR to llout.o and then compile again to get
# the more-complete version of llama.

# LLPAR	= llpar.o
LLPAR	= llout.o

#------------------------------------------------------------
# COMOBJ are objects shared by llama and occs
# LLOBJ  are objects used only by llama
# YYOBJ  are objects used only by occs

COMOBJ = main.o acts.o lexyy.o first.o stok.o
LLOBJ  = llselect.o llcode.o lldriver.o follow.o lldollar.o
YYOBJ  = yypatch.o yycode.o yydriver.o yystate.o yydollar.o

#------------------------------------------------------------

llama:  $(LLPAR) $(LLOBJ) $(COMOBJ) signon.c
	echo \#define __DATE__ \"`date`\" > date.h
	$(CC) -c $(CFLAGS) signon.c
	rm date.h
	$(CC) -o llama $(LLPAR) $(CFLAGS) $(COMOBJ) $(LLOBJ) signon.o \
						$(CLIB) -lcurses -ltermlib

#------------------------------------------------------------

occs:   llout.o $(YYOBJ) $(COMOBJ) signon.c
	echo \#define __DATE__ \"`date`\" >> date.h
	$(CC) -c $(CFLAGS) signon.c
	rm date.h
	$(CC) -o occs llout.o $(COMOBJ) $(YYOBJ) signon.o \
						$(CLIB) -lcurses -ltermlib

# ----------------------------------------------------------------------

acts.o:		acts.pp.c   parser.h
		$(CC) -c $(CFLAGS) acts.pp.c
		mv acts.pp.o acts.o

acts.pp.c:	acts.c
		$(PP) acts.c "$(CFLAGS)"

first.o:	first.c	   parser.h
follow.o:	follow.c   parser.h
main.o:		main.c	   parser.h
stok.o:		stok.c 	   parser.h
yypatch.o:	yypatch.c  parser.h
yycode.o:	yycode.c   parser.h
yydriver.o:	yydriver.c parser.h
yystate.o:	yystate.c  parser.h
yydollar.o:	yydollar.c parser.h

llselect.o:	llselect.c parser.h
llcode.o:	llcode.c   parser.h
lldriver.o:	lldriver.c parser.h
llpar.o:	llpar.c	   parser.h
lldollar.o:	lldollar.c parser.h

llout.o:	llout.c parser.h
lexyy.o:	lexyy.c parser.h

#	The sed call in the following action takes care of an indented #undef
#	in parser.lex. The problem is that UNIX cc requires that the # be in
#	the leftmost column. If you do that in a LeX action, though, then LeX
#	assumes that the current action has ended and you want to recognize
#	a new regular expression (which starts with a #). Fix the problem by
#	indenting the # in parser.lex, but then use sed to left justify the #
#	in lexyy.c.
#
lexyy.c:	parser.lex
		$(LEX) -vl parser.lex
		sed 's/^[ 	]*#[ 	]*undef/#undef/' lexyy.c >.mk.tmp.
		mv .mk.tmp. lexyy.c

llout.c:	parser.lma
		$(LLAMA) -vl $(WIN) parser.lma
