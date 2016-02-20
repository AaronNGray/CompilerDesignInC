/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>

/* SEARCH.C : Implementations and variations on ANSI bsearch.
 *
 * char	*isearch ( key, base, nel, elsize, compare, here )
 * char	*bsearch( key, base, nel, elsize, compare )
 *
 * Bsearch() is Unix compatible, isearch() works like bsearch, but "here" is
 * modified to point at where the element should go in the case of a failure.
 *----------------------------------------------------------------------
 */
void	*isearch( key, base, nel, elsize, compare, put_it_here )
char    *base;
void	*key;
int     nel,  elsize;
int	(* compare)();
void	**put_it_here;
{
    int    low, mid, test;
    char   *current ;

    if ( --nel < 0 )
    {
	*put_it_here = base;
	return NULL ;
    }

    for( low = 0 ; low <= nel ; )
    {
	mid     = ( low + nel ) >> 1 ;			/* (l+h)/2 */
	current = base + (mid * elsize) ;

	if ( (test = (*compare)(key, current)) < 0 )
	    nel = --mid ;

	else if ( test > 0 )
	    low  = ++mid ;
	else
	    return( current );
    }

    *put_it_here = (void *)( test < 0 ? current : current + elsize );
    return NULL;
}
/*----------------------------------------------------------------------*/
void	 *bsearch( key, base, nel, elsize, compare )
char     *base;
void	 *key;
int 	 nel, elsize;
int	 (* compare)();
{
    void	*junk;
    return isearch( key, base, nel, elsize, compare, &junk );
}
/*----------------------------------------------------------------------*/
#ifdef MAIN

cmp( void *key, int *p ){ return( (int)key - *p ); }

main()
{
    static int array[] = { 1, 3, 4, 5,  7,  8 };
    int	  *p, i, *found;
    char   buf[128];

    printf("array is: ");
    for( i = sizeof(array)/sizeof(int), p = array; --i >= 0 ; )
    printf("%d ", *p++ );
    printf("\n");

    while( printf("look for-> "), gets(buf) )
    {
       p = (int *)isearch( (void*)(i=atoi(buf)), array,
					    sizeof(array)/sizeof(int),
					    sizeof(int), cmp, &found );

    /* Must cast the pointer differences to longs in the following
     * printf statments because some compilers evaluate to a long
     * in this situation, and the compiler can't ajust the argument
     * type for a printf argument that corresponds to the ...
     */
    if( !p )
	printf( "%d not found, should be at array[%ld]\n", i,
						    (long)(found-array) );
    else
	printf( "%d found at array[%ld]\n", i, (long)(p-array) );
    }
}
#endif
