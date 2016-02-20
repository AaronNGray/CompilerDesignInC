/*@A (C) 1992 Allen I. Holub                                                */
#include <dos.h>
#include <tools/vbios.h>

int	vb_getchar()
{
    /* Get a character directly from the keyboard */

    union REGS	regs;
    regs.h.ah = 0 ;
    int86( KB_INT, &regs, &regs );
    return( (int)regs.x.ax );
}
