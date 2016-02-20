/*@A (C) 1992 Allen I. Holub                                                */
/* Works like memset but fills integer arrays with an integer value	*/
/* The count is the number of ints (not the number of bytes).  		*/

int	*memiset( dst, with_what, count )
int	*dst, with_what, count;
{
    int *targ;
    for( targ = dst; --count >= 0 ; *targ++ = with_what )
	;
    return dst;
}
