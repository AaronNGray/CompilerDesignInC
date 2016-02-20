/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/debug.h>

/* SSORT.C		Works just like qsort() except that a shell sort, rather
 *			than a quick sort, is used. This is more efficient than
 * quicksort for small numbers of elements, and it's not recursive (so will use
 * much less stack space).
 */

void	ssort( base, nel, elsize, cmp )
char	*base;
int	nel, elsize;
int	(*cmp) P((void*, void*));
{
    int	    i, j;
    int	    gap, k, tmp ;
    char    *p1, *p2;

    for( gap=1; gap <= nel; gap = 3*gap + 1 )
	;

    for( gap /= 3;  gap > 0  ; gap /= 3 )
	for( i = gap; i < nel; i++ )
	    for( j = i-gap; j >= 0 ; j -= gap )
	    {
		p1 = base + ( j      * elsize);
		p2 = base + ((j+gap) * elsize);

		if((*cmp)( (void*)p1, (void*)p2 ) <= 0) /* Compare elements */
		    break;

		for( k = elsize; --k >= 0 ;)	    /* Swap two elements, one */
		{				    /* byte at a time.        */
		    tmp   = *p1;
		    *p1++ = *p2;
		    *p2++ = tmp;
		}
	    }
}

#ifdef MAIN

  cmp( cpp1, cpp2 )
  char	**cpp1, **cpp2;
  {
      register int	rval;
      printf("comparing %s to %s ", *cpp1, *cpp2 );

      rval = strcmp( *cpp1, *cpp2 );

      printf("returning %d\n", rval );
      return rval;
  }

  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

  main( argc, argv )
  int	argc;
  char	**argv;
  {
      ssort( ++argv, --argc, sizeof(*argv), cmp );

      while( --argc >= 0 )
  	printf("%s\n", *argv++ );
  }

#endif
