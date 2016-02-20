/*@A (C) 1992 Allen I. Holub                                                */
   /* ASSORT.C		A version of ssort optimized for arrays of pointers. */
#include <tools/debug.h>

#ifdef __TURBOC__	/* Don't print warning cause elsize isn't used */
#pragma argsused
#endif

void	assort( base, nel, elsize, cmp )
void	**base;
int	nel;
int	elsize;				/* ignored */
int	(*cmp) P((void *, void*));
{
    int	    i, j, gap;
    void    *tmp, **p1, **p2;

    for( gap=1; gap <= nel; gap = 3*gap + 1 )
	;

    for( gap /= 3;  gap > 0  ; gap /= 3 )
	for( i = gap; i < nel; i++ )
	    for( j = i-gap; j >= 0 ; j -= gap )
	    {
		p1 = base + ( j      );
		p2 = base + ((j+gap) );

		if( (*cmp)( p1, p2 ) <= 0 )
		    break;

		tmp = *p1;
		*p1 = *p2;
	        *p2 = tmp;
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
      assort( ++argv, --argc, sizeof(*argv), cmp );

      while( --argc >= 0 )
  	printf("%s\n", *argv++ );
  }

#endif
