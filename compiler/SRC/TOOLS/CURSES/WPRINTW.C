/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"
#include <tools/prnt.h>

PRIVATE int Errcode = OK;

PRIVATE void wputc(c, win)
int	c;
WINDOW	*win;
{
    Errcode |= waddch( win, c );
}

ANSI(  int wprintw( WINDOW *win, char *fmt, ... )	)
KnR (  int wprintw(         win,       fmt      )	)
KnR (  WINDOW	*win;					)
KnR (  char	*fmt;					)
{
    va_list args;
    va_start( args, fmt );
    Errcode = OK;
    prnt( (int (*)()) wputc, win, fmt, args );
    va_end( args );
    return Errcode;
}

ANSI(  int printw( char *fmt, ... )	)
KnR (  int printw(       fmt	 )	)
KnR (  char *fmt;			)
{
    va_list args;
    va_start( args, fmt );

    Errcode = OK;
    prnt( (int (*)()) wputc, stdscr, fmt, args );
    va_end( args );
    return Errcode;
}
