#@A (C) 1992 Allen I. Holub 
#----------------------------------------------------------------------
# This makefile creates four libraries using Borland's C/C++ compiler:
#	comp.lib	is routines used by lex, yacc, and llama, but
#			not by the programs that these programs generate.
#			(small model)
#	compc.lib	Compact-model version of comp.lib
#	l.lib		is the runtime library for lex, yacc, and llama.
#	lc.lib		is a compact-model version of l.lib
#
# You always need to link l.lib to lex, llama, or yacc output files.
#
# In addition, you need to link curses.lib and termlib.lib if you're
# in debug mode. Similarly, the routines used by occs to make the LR(1)
# state machines use the routines in tree.lib.
#
# Note that a few subroutines are in more than one library.
#----------------------------------------------------------------------
# objects in L0 are not included in l.h by the "make l.h" command.

COMP1	 =  assort.obj  bintoasc.obj  copyfile.obj  defnext.obj  driver.obj
CLIB1	 = +assort.obj +bintoasc.obj +copyfile.obj +defnext.obj +driver.obj

COMP2	 =  esc.obj  fputstr.obj  hash.obj  hashadd.obj  hashpjw.obj  mean.obj
CLIB2	 = +esc.obj +fputstr.obj +hash.obj +hashadd.obj +hashpjw.obj +mean.obj

COMP3	 =  memiset.obj  movefile.obj  pairs.obj  pchar.obj  print_ar.obj
CLIB3	 = +memiset.obj +movefile.obj +pairs.obj +pchar.obj +print_ar.obj

COMP4	 = printv.obj searchen.obj set.obj ssort.obj stol.obj
CLIB4	 = +printv.obj +searchen.obj +set.obj +ssort.obj +stol.obj

L0	 =  yymain.obj
LLIB0	 = +yymain.obj

L1	 =  input.obj  yypstk.obj  yywrap.obj  yyhook_a.obj  yyhook_b.obj
LLIB1	 = +input.obj +yypstk.obj +yywrap.obj +yyhook_a.obj +yyhook_b.obj

L2	 =  yyinitlx.obj  yyinitox.obj  yyinitll.obj  yydebug.obj
LLIB2	 = +yyinitlx.obj +yyinitox.obj +yyinitll.obj +yydebug.obj

BOTH	 =  concat.obj  ferr.obj  onferr.obj  prnt.obj
BLIB	 = +concat.obj +ferr.obj +onferr.obj +prnt.obj

COMP_OBJ = $(COMP1) $(COMP2) $(COMP3) $(COMP4) $(BOTH)
L_OBJ	 = $(L0) $(L1) $(L2) $(BOTH)

#----------------------------------------------------------------------
# In my own directory system, some of the sources are in a tools directory
# rather than the current one. Since I've merged these two directories
# together on the distribution disk, both of the following macros are
# the same.
#
# $(ROOT), empty here, is appended to the front of all directory names
# wherever they appear. You should modify it if the compiler-sources file
# system is rooted somewhere else than the actual root directory.

ROOT  =
HERE  = $(ROOT)\src\compiler\lib
TOOLS = $(ROOT)\src\tools

#----------------------------------------------------------------------
# Macro definitions can be overridden from the command line to make
# small-model version. See makelib.sh.

#			To make medium-model libraries (compc.lib lc.lib)
MODEL  = c
CTARG  = \lib\compc
LTARG  = \lib\lc

#			To make small-model libraries (comp.lib l.lib)
# MODEL  = s
# CTARG  = \lib\comp
# LTARG  = \lib\l

CC     = bcc			# Compiler
COMPSW = -c -v -m$(MODEL) -O	# Compiler switches
NO_OPT = -c -v -m$(MODEL)   	# Compiler switches, jump optimization off
#----------------------------------------------------------------------
# creates both libraries

all:	$(LTARG).lib $(CTARG).lib

#----------------------------------------------------------------------

$(CTARG).lib:  $(COMP_OBJ)
	 rm $(CTARG).lib >& nul
	 tlib $(CTARG).lib /C @&&!
$(CLIB1) &
$(CLIB2) &
$(CLIB3) &
$(CLIB4) &
$(BLIB), $(CTARG).ndx
!

#----------------------------------------------------------------------

$(LTARG).lib:  $(L_OBJ)
	 rm $(LTARG).lib >& nul
	 tlib $(LTARG).lib /C @&&!
$(BLIB) &
$(LLIB1) &
$(LLIB2), $(LTARG).ndx
!

#----------------------------------------------------------------------
# The following routines are used by lex and yacc, but not by the
# programs that lex and yacc generate
#

bintoasc.obj:	$(TOOLS)/bintoasc.c
		$(CC) $(COMPSW) $(TOOLS)/bintoasc.c

copyfile.obj:	$(TOOLS)/copyfile.c
		$(CC) $(NO_OPT) $(TOOLS)/copyfile.c

concat.obj:	$(TOOLS)/concat.c
		$(CC) $(COMPSW) $(TOOLS)/concat.c

dmalloc.obj:	$(TOOLS)/dmalloc.c
 		$(CC) $(COMPSW) $(TOOLS)/dmalloc.c

driver.obj:	driver.c
		$(CC) $(COMPSW) $(HERE)/driver.c

defnext.obj:	defnext.c
		$(CC) $(COMPSW) $(HERE)/defnext.c

esc.obj:	$(TOOLS)/esc.c
		$(CC) $(COMPSW) $(TOOLS)/esc.c

ferr.obj:	$(TOOLS)/ferr.c
		$(CC) $(COMPSW) $(TOOLS)/ferr.c

fputstr.obj:	$(TOOLS)/fputstr.c
		$(CC) $(COMPSW) $(TOOLS)/fputstr.c

onferr.obj:	$(TOOLS)/onferr.c
		$(CC) $(COMPSW) $(TOOLS)/onferr.c

hash.obj:	$(TOOLS)/hash.c
		$(CC) $(NO_OPT) $(TOOLS)/hash.c

hashadd.obj:	$(TOOLS)/hashadd.c
		$(CC) $(COMPSW) $(TOOLS)/hashadd.c

hashpjw.obj:	$(TOOLS)/hashpjw.c
		$(CC) $(COMPSW) $(TOOLS)/hashpjw.c

mean.obj:	$(TOOLS)/mean.c
		$(CC) $(COMPSW) $(TOOLS)/mean.c

memiset.obj:	$(TOOLS)/memiset.c
		$(CC) $(COMPSW) $(TOOLS)/memiset.c

movefile.obj:	$(TOOLS)/movefile.c
		$(CC) $(COMPSW) $(TOOLS)/movefile.c

pairs.obj:	pairs.c
		$(CC) $(COMPSW) $(HERE)/pairs.c

print_ar.obj:	print_ar.c
		$(CC) $(COMPSW) $(HERE)/print_ar.c

printv.obj:	$(TOOLS)/printv.c
		$(CC) $(COMPSW) $(TOOLS)/printv.c

prnt.obj:	$(TOOLS)/prnt.c
		$(CC) $(COMPSW) $(TOOLS)/prnt.c

pchar.obj:	$(TOOLS)/pchar.c
		$(CC) $(COMPSW) $(TOOLS)/pchar.c

searchen.obj:	$(TOOLS)/searchen.c
		$(CC) $(COMPSW) $(TOOLS)/searchen.c

set.obj:	$(TOOLS)/set.c
		$(CC) $(COMPSW) $(TOOLS)/set.c

stol.obj:	$(TOOLS)/stol.c
		$(CC) $(COMPSW) $(TOOLS)/stol.c

assort.obj:	$(TOOLS)/assort.c
		$(CC) $(COMPSW) $(TOOLS)/assort.c

ssort.obj:	$(TOOLS)/ssort.c
		$(CC) $(COMPSW) $(TOOLS)/ssort.c

#-----------------------------------------------------------------------
# The objects that follow are the lex and yacc run-time library

input.obj:	input.c
		$(CC) $(NO_OPT) $(HERE)/input.c

yyhook_a.obj:	yyhook_a.c
		$(CC) $(COMPSW) $(HERE)/yyhook_a.c

yyhook_b.obj:	yyhook_b.c
		$(CC) $(COMPSW) $(HERE)/yyhook_b.c

yymain.obj:	yymain.c
		$(CC) $(COMPSW) $(HERE)/yymain.c

yypstk.obj:	yypstk.c
		$(CC) $(COMPSW) $(HERE)/yypstk.c

yywrap.obj:	yywrap.c
		$(CC) $(COMPSW) $(HERE)/yywrap.c

yydebug.obj:	yydebug.c
		$(CC) $(COMPSW) $(HERE)/yydebug.c

yyinitlx.obj:	yyinitlx.c
		$(CC) $(COMPSW) $(HERE)/yyinitlx.c

yyinitox.obj:	yyinitox.c
		$(CC) $(COMPSW) $(HERE)/yyinitox.c

yyinitll.obj:	yyinitll.c
		$(CC) $(COMPSW) $(HERE)/yyinitll.c
