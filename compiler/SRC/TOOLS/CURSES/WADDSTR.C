/*@A (C) 1992 Allen I. Holub                                                */
#include "cur.h"

void waddstr( win, str )
WINDOW	*win;
char	*str;
{
    while( *str )
	waddch( win, *str++ );
}
