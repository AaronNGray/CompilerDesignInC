/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/debug.h>
#include <tools/hash.h>		/* for prototypes only */

#define NBITS_IN_UNSIGNED	(      NBITS(unsigned int)	  )
#if (0 UNIX(+1))
#define SEVENTY_FIVE_PERCENT 	((int)( (NBITS_IN_UNSIGNED * 75 ) /100  ))
#define TWELVE_PERCENT 	((int)( (NBITS_IN_UNSIGNED * 125) /1000 ))
#else
#define SEVENTY_FIVE_PERCENT 	((int)( NBITS_IN_UNSIGNED * .75  ))
#define TWELVE_PERCENT 		((int)( NBITS_IN_UNSIGNED * .125 ))
#endif
#define HIGH_BITS		( ~( (unsigned)(~0) >> TWELVE_PERCENT)  )

unsigned hash_pjw( name )
unsigned char	*name;
{
    unsigned h = 0;			/* Hash value */
    unsigned g;

    for(; *name ; ++name )
    {
	h = (h << TWELVE_PERCENT) + *name ;
	if( g = h & HIGH_BITS )
	    h = (h ^ (g >> SEVENTY_FIVE_PERCENT)) & ~HIGH_BITS ;
    }
    return h;
}
