/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __VIDEO_H
#define __VIDEO_H

#include <tools/debug.h>

#define MONBASE		(DISPLAY FARPTR) 0xb0000000
#define COLBASE		(DISPLAY FARPTR) 0xb8000000
#define NUMROWS		25
#define NUMCOLS		80

typedef	struct
{
    unsigned char letter;
    unsigned char attribute;

} CHARACTER;


typedef CHARACTER	DISPLAY[ NUMROWS ][ NUMCOLS ];


#ifdef ALLOC
	DISPLAY FARPTR dv_Screen = (DISPLAY FARPTR) MONBASE ;
#else
	extern	DISPLAY FARPTR dv_Screen;
#endif

#define SCREEN 		(* dv_Screen)

typedef short		CHAR_ATTRIB;

typedef CHAR_ATTRIB	VDISPLAY[ NUMROWS ][ NUMCOLS ];

#define VSCREEN 	(* (VDISPLAY FARPTR)dv_Screen )

#endif /* __VIDEO_H */
