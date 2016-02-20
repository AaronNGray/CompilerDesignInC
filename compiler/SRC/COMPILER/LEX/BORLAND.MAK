#@A (C) 1992 Allen I. Holub 

# Makefile for creating LeX with Borland C/C++ and the verision of make
# that comes with the compiler.
#
OBJ1 = lex.obj squash.obj minimize.obj dfa.obj input.obj nfa.obj
OBJ2 = print.obj printnfa.obj signon.obj terp.obj

CFLAGS	= -c -v -O -mc
CC	= bcc
BC_OBJ	= c:\bounds\bct4.obj    # Bounds-checker stuff. Set to empty if you
BC_LIB  = c:\bounds\bct.lib	# aren't running bounds checker

.c.obj:
	 $(CC) $(CFLAGS) { $*.c }
#--------------------------------------------------------------
lex.exe:  $(OBJ1) $(OBJ2)
	$(CC) -v -O -mc @&&!
$(OBJ1) $(OBJ2) $(BC_OBJ) \lib\compc.lib $(BC_LIB)
!

final:  $(OBJ1) $(OBJ2)
	$(CC) -O -mc @&&!
$(OBJ1) $(OBJ2) \lib\compc.lib
!
# ----------------------------------------------------------------------
input.obj:	input.c			      globals.h
lex.obj:	lex.c		dfa.h  nfa.h  globals.h
minimize.obj:	minimize.c 	dfa.h 	      globals.h
nfa.obj:	nfa.c		       nfa.h  globals.h
print.obj:	print.c		dfa.h  nfa.h  globals.h
printnfa.obj:	printnfa.c 	       nfa.h
signon.obj:	signon.c
squash.obj:	squash.c 	dfa.h 	      globals.h
dfa.obj:	dfa.c		dfa.h 	      globals.h
terp.obj:	terp.c		nfa.h 	      globals.h
