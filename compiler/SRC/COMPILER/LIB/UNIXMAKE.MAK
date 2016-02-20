#@A (C) 1992 Allen I. Holub 
# This is a version of .../src/compiler/lib/makefile makefile for UNIX.
# Change the "/violet_b/holub/compiler" to root directory of the distribution-
# disk files. You don't have to make any changes to the source-code to recompile
# for Unix. Just type "make".
#
# INCLUDE  is the root directory for various compiler include files (l.h,
#	   compiler.h, etc.) UNIX versions of various ANSI include files are
#	   also in this directory.
# CTARG	   is the full path name of the compiler-support library
# CTARG	   is the full path name of the compiler-runtime library
# CFLAGS   is used by the default .c.obj: dependancy. It is the arguments
#	   sent to the c compiler.
# COMP_OBJ is the list of objects in the compiler-support library
# L_OBJ	   is the list of objects in the compiler-runtime library
#
INCLUDE  = /violet_b/holub/compiler/include
CTARG	 = /violet_b/holub/compiler/lib/comp.lib
LTARG    = /violet_b/holub/compiler/lib/l.lib

CFLAGS  = -I$(INCLUDE)

COMP_OBJ = concat.o ferr.o onferr.o prnt.o strdup.o _sprintf.o \
	   assort.o bintoasc.o copyfile.o defnext.o driver.o esc.o \
	   fputstr.o hash.o hashadd.o hashpjw.o mean.o memiset.o \
	   movefile.o pairs.o pchar.o print_ar.o printv.o search.o \
	   searchen.o set.o ssort.o stol.o

L_OBJ	 = concat.o ferr.o onferr.o prnt.o strdup.o _sprintf.o\
	   input.o yymain.o yypstk.o yywrap.o yyhook_a.o yyhook_b.o \
	   yyinitlx.o yyinitox.o yyinitll.o yydebug.o

#----------------------------------------------------------------------
all:	$(LTARG) $(CTARG)

$(CTARG):	$(COMP_OBJ)
	rm $(CTARG)
	ar rv $(CTARG) $(COMP_OBJ)
	ranlib $(CTARG)

$(LTARG):	$(L_OBJ)
	rm $(LTARG)
	ar rv $(LTARG) $(L_OBJ)
	ranlib $(LTARG)

#----------------------------------------------------------------------
# The following routines are used by lex and yacc, but not by the
# programs that lex and yacc generate
#

bintoasc.o:	bintoasc.c
copyfile.o:	copyfile.c
concat.o:	concat.c
dmalloc.o:	dmalloc.c
driver.o:	driver.c
defnext.o:	defnext.c
esc.o:		esc.c
ferr.o:		ferr.c
fputstr.o:	fputstr.c
onferr.o:	onferr.c
hash.o:		hash.c
hashadd.o:	hashadd.c
hashpjw.o:	hashpjw.c
mean.o:		mean.c
memiset.o:	memiset.c
movefile.o:	movefile.c
pairs.o:	pairs.c
print_ar.o:	print_ar.c
printv.o:	printv.c
prnt.o:		prnt.c
pchar.o:	pchar.c
search.o:	search.c
searchen.o:	searchen.c
set.o:		set.c
_sprintf.o:	_sprintf.c
stol.o:		stol.c
assort.o:	assort.c
ssort.o:	ssort.c
strdup.o:	strdup.c

#-----------------------------------------------------------------------
# The objects that follow are the lex and yacc run-time library

input.o:	input.c
yyhook_a.o:	yyhook_a.c
yyhook_b.o:	yyhook_b.c
yymain.o:	yymain.c
yypstk.o:	yypstk.c
yywrap.o:	yywrap.c
yydebug.o:	yydebug.c
yyinitlx.o:	yyinitlx.c
yyinitox.o:	yyinitox.c
yyinitll.o:	yyinitll.c
