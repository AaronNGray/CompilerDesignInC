#@A (C) 1992 Allen I. Holub 

# Makefile to create llama or occs using Borland C/C++ and
# the version of make that comes with the compiler.
#------------------------------------------------------------
# Uncomment:
#
# WIN = -D	# uncomment to include the IDE in llama/occs itself
#
CFLAGS	= -v -O
CC	= bcc
BC_OBJ	= c:\bounds\bct4.obj    # Bounds-checker stuff. Set to empty if you
BC_LIB  = c:\bounds\bct.lib	# aren't running bounds checker

#------------------------------------------------------------
# Use this makefile to manufacture occs as follows:
#
# (1) Make a recursive-descent version of llama by uncommenting the
#     first group of definitions below. Also, LLPAR = llpar.obj
#     definition. Run make.
#
# (2) Make a full implementation of llama by changing the LLPAR
#     definition to LLPAR=llout.obj. Run make again.
#
# (3) Make occs by commenting out the first block of definitions (the
#     one used to make llama), and then uncommenting the "OCCS, Medium
#     model" definitions just below. Run make a third time. The
#     "Occs, Small model" is useful only for debugging. A small-model
#     version of occs can't handle realistic grammars.

#				Compile LLAMA, Small model is sufficient
PROG	= -DLLAMA
MODEL	= -ms
CLIB	= comp.lib l.lib
LLPAR	= llpar.obj
#			LLPAR	= llout.obj
target: llama.exe

#				Compile OCCS, Medium model
# PROG	 = -DOCCS
# MODEL  = -mc
# CLIB	 = compc.lib lc.lib cursesc.lib termlibc.lib
# target: occs.exe

#			Compile OCCS, Small model (for debugging occs)
# PROG 	 = -DOCCS
# MODEL  = -ms
# CLIB	 = comp.lib l.lib curses.lib termlib.lib
# target: occs.exe

# ============================================================
# COMOBJ are objects shared by llama and occs
# LLOBJ  are objects used only by llama
# YYOBJ  are objects used only by occs

COMOBJ = main.obj      acts.obj    lexyy.obj     first.obj    stok.obj
LLOBJ  = llselect.obj  llcode.obj  lldriver.obj  follow.obj   lldollar.obj
YYOBJ  = yypatch.obj   yycode.obj  yydriver.obj  yystate.obj  yydollar.obj

# ------------------------------------------------------------

.c.obj:
	 $(CC) $(PROG) -c $(CFLAGS) $(MODEL) { $*.c }

#------------------------------------------------------------

llama.exe:  $(LLPAR) $(LLOBJ) $(COMOBJ) signon.c
	$(CC) $(PROG) -c $(CFLAGS) $(MODEL) signon.c
	$(CC) $(PROG)    $(CFLAGS) $(MODEL) @&&!
signon.obj $(LLPAR) $(LLOBJ) $(COMOBJ) $(BC_OBJ) $(CLIB) $(BC_LIB)
!
    mv signon.exe llama.exe

#------------------------------------------------------------

occs.exe:   llout.obj $(YYOBJ) $(COMOBJ) signon.c
	$(CC) $(PROG) -c $(CFLAGS) $(MODEL) signon.c
	$(CC) $(PROG)    $(CFLAGS) $(MODEL) @&&!
signon.obj llout.obj $(YYOBJ) $(COMOBJ) $(BC_OBJ) $(CLIB) $(BC_LIB)
!
    mv signon.exe occs.exe

# ----------------------------------------------------------------------

first.obj:	first.c	   parser.h
follow.obj:	follow.c   parser.h
acts.obj:	acts.c	   parser.h
main.obj:	main.c	   parser.h
stok.obj:	stok.c 	   parser.h
yypatch.obj:	yypatch.c  parser.h
yycode.obj:	yycode.c   parser.h
yydriver.obj:	yydriver.c parser.h
yystate.obj:	yystate.c  parser.h
yydollar.obj:	yydollar.c parser.h

llselect.obj:	llselect.c parser.h
llcode.obj:	llcode.c   parser.h
lldriver.obj:	lldriver.c parser.h
llpar.obj:	llpar.c	   parser.h
lldollar.obj:	lldollar.c parser.h

llout.obj:	llout.c parser.h
lexyy.obj:	lexyy.c parser.h
lexyy.c:	parser.lex
		lex -vl parser.lex

llout.c:	parser.lma
		llama -vl $(WIN) parser.lma
