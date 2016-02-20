/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>
#include <tools/compiler.h>

void	pchar(c, stream)
int	c;
FILE	*stream;
{
    fputs( bin_to_ascii( c, 1 ), stream );
}
