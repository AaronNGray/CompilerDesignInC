#@A (C) 1992 Allen I. Holub 
#----------------------------------------------------------------------
# Make the compiler under UNIX.
#----------------------------------------------------------------------

CFLAGS = -I/violet_b/holub/compiler/include
OCCS   = /violet_b/holub/compiler/bin/occs
LEX    = /violet_b/holub/compiler/bin/LeX
PP     = /violet_b/holub/compiler/bin/pp

#----------------------------------------------------------------------
OBJ = 	decl.o gen.o lexyy.o local.o main.o op.o switch.o \
	symtab.o temp.o value.o yyact.o yyout.o yyoutab.o

LIB =	/violet_b/holub/compiler/lib/l.lib \
	/violet_b/holub/compiler/lib/comp.lib -lcurses -ltermlib

#----------------------------------------------------------------------

c:	$(OBJ)
	cc -o c $(CFLAGS) $(OBJ) $(LIB)

#----------------------------------------------------------------------
yyoutab.o:	yyoutab.c
yyout.o:	yyout.c

# yyout.c is deliberatly not in the make heirarchy becaust it takes a while
# for occs to produce it and you only have to remake it if you change the
# grammar (changing actions does not affect yyout.c. Remake it explicitly
# when you need to.

_yyout.c:	c.y
		$(OCCS) -vlWDSp c.y

#----------------------------------------------------------------------
yyact.o:	yyact.pp.c proto.h symtab.h value.h
		$(CC) -c $(CFLAGS) yyact.pp.c
		mv yyact.pp.o yyact.o

yyact.pp.c:	yyact.c
		$(PP) yyact.c "$(CFLAGS)"

yyact.c:	c.y
		$(OCCS) -vWDla c.y

lexyy.o:	lexyy.c symtab.h yyout.h
lexyy.c:	c.lex
		$(LEX) -vl c.lex

decl.o:		decl.c   symtab.h value.h proto.h
gen.o:		gen.c    		  proto.h
local.o:	local.c  symtab.h	  proto.h label.h
main.o:		main.c 			  proto.h
op.o:		op.c     symtab.h value.h proto.h label.h
switch.o:	switch.c symtab.h value.h proto.h label.h switch.h
symtab.o:	symtab.c symtab.h value.h proto.h label.h
temp.o:		temp.c   symtab.h value.h proto.h
value.o:	value.c  symtab.h value.h proto.h label.h
