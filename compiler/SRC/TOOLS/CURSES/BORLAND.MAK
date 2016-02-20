#@A (C) 1992 Allen I. Holub 

OBJ1 =  box.obj     delwin.obj   glue.obj    hidewin.obj  initscr.obj
OBJ2 =  mvwin.obj   showwin.obj  waddstr.obj wclrtoeo.obj werase.obj
OBJ3 =  winch.obj   wincreat.obj winio.obj   wmove.obj    wprintw.obj
OBJ4 =  wscroll.obj prnt.obj

LIB1 = +box.obj     +delwin.obj   +glue.obj    +hidewin.obj +initscr.obj
LIB2 = +mvwin.obj   +showwin.obj  +waddstr.obj +wclrtoeo.obj +werase.obj
LIB3 = +winch.obj   +wincreat.obj +winio.obj   +wmove.obj    +wprintw.obj
LIB4 = +wscroll.obj +prnt.obj
# ----------------------------------------------------------------------
# You'll have to recompile glue.c if you change the TYPE macro.
# You'll have to recompile everything to go from small to compact model.
#
# TYPE	= R	# make rom-bios version of curses
# TYPE	= V	# make direct-video version of curses
TYPE	= A	# make autoselect version, decides based on VIDEO environment

# MODEL = c	# compact model
MODEL = s	# small model

SW	= -c -O -v -m$(MODEL)		# Compiler switches
TARG	= curses$(TYPE)$(MODEL)		# target-library name
CC	= bcc				# compiler
# ----------------------------------------------------------------------
.c.obj:
	$(CC) $(SW) { \src\tools\curses\$*.c }
# ----------------------------------------------------------------------
$(TARG).lib: $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4)
	 rm $(TARG).lib
	 tlib $(TARG).lib /C @&&!
$(LIB1) &
$(LIB2) &
$(LIB3) &
$(LIB4), $(TARG).ndx
!
# ----------------------------------------------------------------------
PRNT=..
prnt.obj:	$(PRNT)\prnt.c
		$(CC) $(SW) $(PRNT)\prnt.c

glue.obj:	glue.c
		$(CC) $(SW) -D$(TYPE) \src\tools\curses\glue.c

box.obj:	box.c		cur.h /include/curses.h
delwin.obj:	delwin.c	cur.h /include/curses.h
hidewin.obj:	hidewin.c	cur.h /include/curses.h
initscr.obj:	initscr.c	cur.h /include/curses.h
mvwin.obj:	mvwin.c		cur.h /include/curses.h
showwin.obj:	showwin.c	cur.h /include/curses.h
waddstr.obj:	waddstr.c	cur.h /include/curses.h
wclrtoeo.obj:	wclrtoeo.c	cur.h /include/curses.h
werase.obj:	werase.c	cur.h /include/curses.h
winch.obj:	winch.c		cur.h /include/curses.h
wincreat.obj:	wincreat.c	cur.h /include/curses.h
winio.obj:	winio.c		cur.h /include/curses.h
wmove.obj:	wmove.c		cur.h /include/curses.h
wprintw.obj:	wprintw.c	cur.h /include/curses.h
wscroll.obj:	wscroll.c	cur.h /include/curses.h
