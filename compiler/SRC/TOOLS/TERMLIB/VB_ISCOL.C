/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/vbios.h>

int vb_iscolor()   		/* Returns true if a color card is active */
{
    int	mode = _Vbios( GET_VMODE, 0, 0, 0, 0, "ax" ) & 0xff ;
    return( (mode == 7 		   ) ?  0  :
    	    (mode == 2 || mode == 3) ?  1  :  -1 );
}
