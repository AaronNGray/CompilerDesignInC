/*@A (C) 1992 Allen I. Holub                                                */
#include <dos.h>
#include <tools/vbios.h>
#include "video.h"

/* This file contains a workhorse function used by the other video I/O	*/
/* functions to talk to the BIOS. It executes the video interrupt.  	*/

int	_Vbios( service, al, bx, cx, dx, return_this )
int	service;		/* Service code, put into ah.		      */
int	al, bx, cx, dx;		/* Other input registers.		      */
char	*return_this;		/* Register to return, "ax", "bh", "dx" only. */
{
    union REGS	regs;

    regs.h.ah = service;
    regs.h.al = al;
    regs.x.bx = bx;
    regs.x.cx = cx;
    regs.x.dx = dx;
    int86( VIDEO_INT, &regs, &regs);
    return ( *return_this == 'a' ) ?  regs.x.ax :
           ( *return_this == 'b' ) ?  regs.h.bh : regs.x.dx ;
}
