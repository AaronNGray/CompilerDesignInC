/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/l.h>
#include <tools/compiler.h>
#include <tools/c-code.h>
#include "symtab.h"
#include "value.h"
#include "proto.h"

/* Subroutines in this file take care of temporary-variable management.  */

#define	REGION_MAX 128	/* Maximum number of stack elements that can be	*/
			/* used for temporaries.			*/
#define MARK	-1	/* Marks cells that are in use but are not the	*/
			/* first cell of the region.			*/
typedef int CELL;	/* In-use (Region) map is an array of these.	*/


PRIVATE CELL	Region[REGION_MAX];
PRIVATE CELL	*High_water_mark = Region;
/*----------------------------------------------------------------------*/
PUBLIC int tmp_alloc( size )
int	size;				/* desired number of bytes */
{
    /* Allocate a portion of the temporary-variable region of the required size,
     * expanding the tmp-region size if necessary. Return the offset in bytes
     * from the start of the rvalue region to the first cell of the temporary.
     * 0 is returned if no space is available, and an error message is also
     * printed in this situation. This way the code-generation can go on as if
     * space had been found without having to worry about testing for errors.
     * (Bad code is generated, but so what?)
     */

    CELL  *start, *p ;
    int	  i;

    /* size = the number of stack cells required to hold "size" bytes.  */

    size = ((size + SWIDTH) / SWIDTH)  -  (size % SWIDTH == 0);

    if( !size )
	yyerror("INTERNAL, tmp_alloc: zero-length region requested\n" );

    /* Now look for a large-enough hole in the already-allocated cells.  */

    for( start = Region; start < High_water_mark ;)
    {
	for( i = size, p = start;  --i >= 0  &&  !*p;  ++p )
		;

	if( i >= 0 ) 				  	   /* Cell not found. */
	    start = p + 1;
	else					 /* Found an area big enough. */
	    break;
    }

    if( start < High_water_mark )	         /* Found a hole. */
	p  = start;
    else
    {
	if( (High_water_mark + size) > (Region + REGION_MAX) )    /* No room. */
	{
	    yyerror("Expression too complex, break into smaller pieces\n");
	    return 0;
	}
	p = High_water_mark;
	High_water_mark += size;
    }

    for( *p = size; --size > 0; *++p = MARK) /* 1st cell=size. Others=MARK */
	;

    return (int)(start - Region);  	  /* Return offset to start of region */
					  /* converted to bytes.	      */
}
/*----------------------------------------------------------------------*/
PUBLIC	void tmp_free( offset )
int  offset;			   /* Release a temporary var.; offset should */
{				   /* have been returned from tmp_alloc().    */
    CELL  *p  = Region + offset;
    int	  size;

    if( offset < 0 )
	yyerror( "INTERNAL, tmp_free: Bad offset (%d)\n", offset );

    else if( p > High_water_mark )
	yyerror( "INTERNAL, tmp_free: (offset=%d) > High-water mark (%d)\n",
				       offset, (int)(High_water_mark - Region));
    else if( !*p )
	yyerror( "INTERNAL, tmp_free: no size\n" );

    else if( *p == MARK )
	yyerror( "INTERNAL, tmp_free: size == MARK\n" );

    else
	for( size = *p; --size >= 0; *p++ = 0 )
		;
}
/*----------------------------------------------------------------------*/
PUBLIC	void	tmp_reset()
{
    /* Reset everything back to the virgin state, including the high-water mark.
     * This routine should be called just before a subroutine body is processed,
     * when the prefix is output. See also: tmp_freeall().
     */
    tmp_freeall();
    High_water_mark = Region ;
}
/*----------------------------------------------------------------------*/
PUBLIC	void	tmp_freeall()
{
    /* Free all temporaries currently in use (without modifying the high-water
     * mark). This subroutine should be called after processing arithmetic
     * statements to clean up any temporaries still kicking around (there is
     * usually at least one).
     */
    memset( Region, 0, sizeof(Region) );
}
/*----------------------------------------------------------------------*/
PUBLIC	int	tmp_var_space()
{
    /* Return the total cumulative size of the temporary-variable region in
     * stack elements, not bytes. This number can be used as an argument to the
     * link instruction.
     */
     return (int)( High_water_mark - Region );
}
