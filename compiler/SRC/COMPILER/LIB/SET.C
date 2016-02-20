/*@A (C) 1992 Allen I. Holub                                                */

#include  <stdio.h>
#include  <ctype.h>
#include  <signal.h>
#include  <stdlib.h>
#include  <string.h>
#include  <tools/debug.h>
#include  <tools/set.h>

PUBLIC SET	*newset()
{
    /* Create a new set and return a pointer to it. Print an error message
     * and raise SIGABRT if there's insufficient memory. NULL is returned
     * if raise() returns.
     */

    SET	*p;

    if( !(p = (SET *) malloc( sizeof(SET) )) )
    {
	fprintf(stderr,"Can't get memory to create set\n");
	raise( SIGABRT );
	return NULL;		/* Usually won't get here */
    }
    memset( p, 0, sizeof(SET) );
    p->map    = p->defmap;
    p->nwords = _DEFWORDS;
    p->nbits  = _DEFBITS;
    return p;
}

/*----------------------------------------------------------------------*/

PUBLIC void	delset( set )
SET	*set;
{
    /* Delete a set created with a previous newset() call. */

    if( set->map != set->defmap )
	free( set->map );
    free( set );
}

/*----------------------------------------------------------------------*/

PUBLIC SET	*dupset( set )
SET	*set;
{
    /* Create a new set that has the same members as the input set */

    SET	  *new;

    if( !(new = (SET *) malloc( sizeof(SET) )) )
    {
	fprintf(stderr,"Can't get memory to duplicate set\n");
	exit(1);
    }

    memset( new, 0, sizeof(SET) );
    new->compl  = set->compl;
    new->nwords = set->nwords;
    new->nbits  = set->nbits;

    if( set->map == set->defmap )		   /* default bit map in use */
    {
	new->map = new->defmap;
	memcpy( new->defmap, set->defmap, _DEFWORDS * sizeof(_SETTYPE) );
    }
    else					/* bit map has been enlarged */
    {
	new->map = (_SETTYPE *) malloc( set->nwords * sizeof(_SETTYPE) );
	if( !new->map )
	{
	    fprintf(stderr,"Can't get memory to duplicate set bit map\n");
	    exit(1);
	}
	memcpy( new->map, set->map, set->nwords * sizeof(_SETTYPE) );
    }
    return new;
}

PUBLIC int	_addset( set, bit )
SET	*set;
int	bit;
{
    /* Addset is called by the ADD() macro when the set isn't big enough. It
     * expands the set to the necessary size and sets the indicated bit.
     */

    void enlarge P(( int, SET* ));		/* immediately following */

    enlarge( _ROUND(bit), set );
    return _GBIT( set, bit, |= );
}
/* ------------------------------------------------------------------- */
PRIVATE	void	enlarge( need, set )
int	need;
SET	*set;
{
    /* Enlarge the set to "need" words, filling in the extra words with zeros.
     * Print an error message and exit if there's not enough memory.
     * Since this routine calls malloc, it's rather slow and should be
     * avoided if possible.
     */

    _SETTYPE  *new;

    if( !set  ||  need <= set->nwords )
	return;

    D( printf("enlarging %d word map to %d words\n", set->nwords, need); )

    if( !(new = (_SETTYPE *) malloc( need * sizeof(_SETTYPE)))  )
    {
	fprintf(stderr, "Can't get memory to expand set\n");
	exit( 1 );
    }
    memcpy( new, set->map,        set->nwords * sizeof(_SETTYPE)          );
    memset( new + set->nwords, 0, (need - set->nwords) * sizeof(_SETTYPE) );

    if( set->map != set->defmap )
	free( set->map );

    set->map    = new  ;
    set->nwords = (unsigned char) need ;
    set->nbits  = need * _BITS_IN_WORD ;
}

PUBLIC int	num_ele( set )
SET	*set;
{
    /* Return the number of elements (nonzero bits) in the set. NULL sets are
     * considered empty. The table-lookup approach used here was suggested to
     * me by Doug Merrit. Nbits[] is indexed by any number in the range 0-255,
     * and it evaluates to the number of bits in the number.
     */
    static unsigned char nbits[] =
    {
	/*   0-15  */	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
	/*  16-31  */	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	/*  32-47  */	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	/*  48-63  */	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	/*  64-79  */	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	/*  80-95  */	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	/*  96-111 */	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	/* 112-127 */	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	/* 128-143 */	1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	/* 144-159 */	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	/* 160-175 */	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	/* 176-191 */	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	/* 192-207 */	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	/* 208-223 */	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	/* 224-239 */	3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	/* 240-255 */	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
    };
    int			i;
    unsigned int 	count = 0;
    unsigned char	*p;

    if( !set )
	return 0;

    p = (unsigned char *)set->map ;

    for( i = _BYTES_IN_ARRAY(set->nwords) ; --i >= 0 ; )
	count += nbits[ *p++ ] ;

    return count;
}

/* ------------------------------------------------------------------- */

PUBLIC int	_set_test( set1, set2 )
SET	*set1, *set2;
{
    /* Compares two sets. Returns as follows:
     *
     * _SET_EQUIV	Sets are equivalent
     * _SET_INTER	Sets intersect but aren't equivalent
     * _SET_DISJ	Sets are disjoint
     *
     * The smaller set is made larger if the two sets are different sizes.
     */

    int	  	i, rval = _SET_EQUIV ;
    _SETTYPE   *p1, *p2;

    i = max( set1->nwords, set2->nwords);

    enlarge( i, set1 );		/* Make the sets the same size	*/
    enlarge( i, set2 );

    p1 = set1->map;
    p2 = set2->map;

    for(; --i >= 0 ; p1++, p2++ )
    {
	if( *p1 != *p2 )
	{
	    /* You get here if the sets aren't equivalent. You can return
	     * immediately if the sets intersect but have to keep going in the
	     * case of disjoint sets (because the sets might actually intersect
	     * at some byte, as yet unseen).
	     */

	    if( *p1 & *p2 )
		return _SET_INTER ;
	    else
		rval = _SET_DISJ ;
	}
    }

    return rval;			/* They're equivalent	*/
}

/* ------------------------------------------------------------------- */

PUBLIC int setcmp( set1, set2 )
SET *set1, *set2;
{
    /* Yet another comparison function. This one works like strcmp(),
     * returning 0 if the sets are equivalent, <0 if set1<set2 and >0 if
     * set1>set2. A NULL set is less than any other set, two NULL sets
     * are equivalent.
     */

    int	  i, j;
    _SETTYPE   *p1, *p2;

    D( if( !set1 ) fprintf(stderr, "setcmp(): set1 is NULL\n" ); )
    D( if( !set2 ) fprintf(stderr, "setcmp(): set2 is NULL\n" ); )

    if(  set1 ==  set2 ){ return  0; }
    if(  set1 && !set2 ){ return  1; }
    if( !set1 &&  set2 ){ return -1; }

    i = j = min( set1->nwords, set2->nwords );

    for( p1 = set1->map, p2 = set2->map ; --j >= 0 ;  p1++, p2++  )
	if( *p1 != *p2 )
	    return *p1 - *p2;

    /* You get here only if all words that exist in both sets are the same.
     * Check the tail end of the larger array for all zeros.
     */

    if( (j = set1->nwords - i) > 0 )		/* Set 1 is the larger */
    {
	while( --j >= 0 )
	    if( *p1++ )
		return 1;
    }
    else if( (j = set2->nwords - i) > 0)	/* Set 2 is the larger */
    {
	while( --j >= 0 )
	    if( *p2++ )
		return -1;
    }

    return 0;					/* They're equivalent	*/
}

/* ------------------------------------------------------------------- */

PUBLIC unsigned sethash( set1 )
SET *set1;
{
    /* hash the set by summing together the words in the bit map */

    _SETTYPE    *p;
    unsigned	total;
    int		j;

    total = 0;
    j     = set1->nwords ;
    p     = set1->map    ;

    while( --j >= 0 )
	total += *p++ ;

    return total;
}

/* ------------------------------------------------------------------- */

PUBLIC int	subset( set, possible_subset )
SET	*set, *possible_subset;
{
    /* Return 1 if "possible_subset" is a subset of "set". One is returned if
     * it's a subset, zero otherwise. Empty sets are subsets of everything.
     * The routine silently malfunctions if given a NULL set, however. If the
     * "possible_subset" is larger than the "set", then the extra bytes must
     * be all zeros.
     */

    _SETTYPE  *subsetp, *setp;
    int	  common;		/* This many bytes in potential subset  */
    int	  tail;			/* This many implied 0 bytes in b	*/

    if( possible_subset->nwords > set->nwords )
    {
	common = set->nwords ;
	tail   = possible_subset->nwords - common ;
    }
    else
    {
	common = possible_subset->nwords;
	tail   = 0;
    }

    subsetp = possible_subset->map;
    setp    = set->map;

    for(; --common >= 0; subsetp++, setp++ )
	if( (*subsetp & *setp) != *subsetp )
	    return 0;

    while( --tail >= 0 )
	if( *subsetp++ )
	    return 0;

    return 1;
}

PUBLIC void	_set_op( op, dest, src )
int	op;
SET	*src, *dest;
{
    /* Performs binary operations depending on op:
     *
     * _UNION:        dest = union of src and dest
     * _INTERSECT:    dest = intersection of src and dest
     * _DIFFERENCE:   dest = symmetric difference of src and dest
     * _ASSIGN:       dest = src;
     *
     * The sizes of the destination set is adjusted so that it's the same size
     * as the source set.
     */

    _SETTYPE	*d;	/* Pointer to destination map	*/
    _SETTYPE	*s;	/* Pointer to map in set1	*/
    char	ssize;	/* # of words in src set	*/
    int		tail;	/* dest set is this much bigger */

    ssize = src->nwords ;

    if( dest->nwords < ssize ) /* Make sure dest set is at least */
	enlarge( ssize, dest );		 /* as big as the src set.	   */

    tail  = dest->nwords - ssize ;
    d     = dest->map ;
    s     = src ->map ;

    switch( op )
    {
    case _UNION:      while( --ssize >= 0 )
			    *d++ |= *s++ ;
		      break;
    case _INTERSECT:  while( --ssize >= 0 )
			    *d++ &= *s++ ;
		      while( --tail >= 0 )
			    *d++ = 0;
		      break;
    case _DIFFERENCE: while( --ssize >= 0 )
			    *d++ ^= *s++ ;
		      break;
    case _ASSIGN:     while( --ssize >= 0 )
			    *d++  = *s++ ;
		      while( --tail >= 0 )
			    *d++ = 0;
		      break;
    }
}

/* ------------------------------------------------------------------- */

PUBLIC void	invert( set )
SET	*set;
{
    /* Physically invert the bits in the set. Compare with the COMPLEMENT()
     * macro, which just modifies the complement bit.
     */

    _SETTYPE *p, *end ;

    for( p = set->map, end = p + set->nwords ; p < end ; p++ )
	*p = ~*p;
}

/* ------------------------------------------------------------------- */

PUBLIC void	truncate( set )
SET	*set;
{
    /* Clears the set but also set's it back to the original, default size.
     * Compare this routine to the CLEAR() macro which clears all the bits in
     * the map but doesn't modify the size.
     */

    if( set->map != set->defmap )
    {
	free( set->map );
	set->map = set->defmap;
    }
    set->nwords = _DEFWORDS;
    set->nbits  = _DEFBITS;
    memset( set->defmap, 0, sizeof(set->defmap) );
}

PUBLIC int	next_member( set )
SET	*set;
{
    /* set == NULL			Reset
     * set changed from last call:	Reset and return first element
     * otherwise			return next element or -1 if none.
     */

    static SET	*oset 	       = NULL;	/* "set" arg in last call 	    */
    static int	current_member = 0;	/* last-accessed member of cur. set */
    _SETTYPE    *map;

    if( !set )
	return( (int)( oset = NULL ) );

    if( oset != set )
    {
	oset           = set;
	current_member = 0 ;

	for(map = set->map; *map == 0  &&  current_member < set->nbits; ++map)
	    current_member += _BITS_IN_WORD;
    }

    /* The increment must be put into the test because, if the TEST() invocation
     * evaluates true, then an increment on the right of a for() statement
     * would never be executed.
     */

    while( current_member++ < set->nbits )
	if( TEST(set, current_member-1) )
	    return( current_member-1 );
    return( -1 );
}

/* ------------------------------------------------------------------- */

PUBLIC void	pset( set, output_routine, param )
SET	*set;
pset_t	output_routine;
void	*param;
{
    /* Print the contents of the set bit map in human-readable form. The
     * output routine is called for each element of the set with the following
     * arguments:
     *
     * (*out)( param, "null",    -1);	Null set ("set" arg == NULL)
     * (*out)( param, "empty",   -2);	Empty set (no elements)
     * (*out)( param, "%d ",      N);	N is an element of the set
     */

    int	i, did_something = 0;

    if( !set )
	(*output_routine)( param, "null", -1 );
    else
    {
	next_member( NULL );
	while( (i = next_member(set))  >= 0 )
	{
	    did_something++;
	    ( *output_routine )( param, "%d ", i  );
	}
	next_member( NULL );

	if( !did_something )
	    ( *output_routine )(param, "empty",  -2 );
    }
}
#ifdef MAIN

scmp(a,b) SET **a, **b; { return setcmp(*a,*b); }

main()
{
    int i;
    SET *s1 = newset();
    SET *s2 = newset();
    SET *s3 = newset();
    SET *s4 = newset();
    SET *a[ 40 ];

    printf("adding 1024 and 2047 to s1: ");
    ADD(  s2,1024);
    ADD(  s2,2047);
    pset( s2, (pset_t) fprintf, stdout );
    printf("removing 1024 and 2047: ");
    REMOVE(  s2, 1024);
    REMOVE(  s2, 2047);
    pset( s2, (pset_t) fprintf, stdout );

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    for( i = 0; i <= 1024 ; ++i )
    {
	ADD( s1, i );
	if( !TEST(s1,i) || !MEMBER(s1,i) )
	    printf("initial: <%d> not in set and it should be\n", i);
    }
    for( i = 0; i <= 1024 ; ++i )
    {
	/* Make a second pass to see if a previous ADD messed
	 * up an already-added element.
	 */

	if( !TEST(s1,i) || !MEMBER(s1,i) )
	    printf("verify:  <%d> not in set and it should be\n", i);
    }
    for( i = 0; i <= 1024 ; ++i )
    {
	REMOVE( s1, i );
	if( TEST(s1,i) || MEMBER(s1,i) )
	    printf("initial: <%d> is in set and it shouldn't be\n", i);
    }
    for( i = 0; i <= 1024 ; ++i )
    {
	if( TEST(s1,i) || MEMBER(s1,i) )
	    printf("verify:  <%d> is in set and it shouldn't be\n", i);
    }

    printf("Add test finished: malloc set\n" );

    /*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

    truncate(s1);

    printf(  IS_EQUIVALENT(s1,s2) ? "yeah!\n" : "boo\n" );

    ADD( s1, 1 );
    ADD( s2, 1 );
    ADD( s3, 1 );
    ADD( s1, 517 );
    ADD( s2, 517 );

    printf(  IS_EQUIVALENT(s1,s2) ? "yeah!\n" : "boo\n" );

    REMOVE( s2, 517 );

    printf( !IS_EQUIVALENT(s1,s2) ? "yeah!\n" : "boo\n" );
    printf( !IS_EQUIVALENT(s2,s1) ? "yeah!\n" : "boo\n" );
    printf( !IS_EQUIVALENT(s1,s3) ? "yeah!\n" : "boo\n" );
    printf(  IS_EQUIVALENT(s3,s2) ? "yeah!\n" : "boo\n" );
    printf(  IS_EQUIVALENT(s2,s3) ? "yeah!\n" : "boo\n" );

    /*-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  */

    ADD(s1, 3);
    ADD(s1, 6);
    ADD(s1, 9);
    ADD(s1, 12);
    ADD(s1, 15);
    ADD(s1, 16);
    ADD(s1, 19);
    ADD(s3, 18);

    printf(   "s1=" );  pset( s1, (pset_t) fprintf, stdout );
    printf( "\ns2=" );  pset( s2, (pset_t) fprintf, stdout );
    printf( "\ns3=" );  pset( s3, (pset_t) fprintf, stdout );
    printf( "\ns4=" );  pset( s4, (pset_t) fprintf, stdout );
    printf( "\n"    );
    printf("s1 has %d elements\n", num_ele( s1 ) );
    printf("s2 has %d elements\n", num_ele( s2 ) );
    printf("s3 has %d elements\n", num_ele( s3 ) );
    printf("s4 has %d elements\n", num_ele( s4 ) );

    s2 = dupset( s1 );
    printf( IS_EQUIVALENT(s2,s1)? "dupset succeeded\n" : "dupset failed\n");

    printf( "\ns1 %s empty\n", IS_EMPTY(s1) ? "IS" : "IS NOT" );
    printf( "s3 %s empty\n",   IS_EMPTY(s3) ? "IS" : "IS NOT" );
    printf( "s4 %s empty\n",   IS_EMPTY(s4) ? "IS" : "IS NOT" );

    printf("s1&s3 %s disjoint\n", IS_DISJOINT    (s1,s3) ? "ARE":"ARE NOT");
    printf("s1&s4 %s disjoint\n", IS_DISJOINT    (s1,s4) ? "ARE":"ARE NOT");

    printf("s1&s3 %s intersect\n",IS_INTERSECTING(s1,s3) ? "DO" : "DO NOT");
    printf("s1&s4 %s intersect\n",IS_INTERSECTING(s1,s4) ? "DO" : "DO NOT");

    printf("s1 %s a subset of s1\n", subset(s1,s1) ? "IS" : "IS NOT" );
    printf("s3 %s a subset of s3\n", subset(s3,s3) ? "IS" : "IS NOT" );
    printf("s4 %s a subset of s4\n", subset(s4,s4) ? "IS" : "IS NOT" );
    printf("s1 %s a subset of s3\n", subset(s3,s1) ? "IS" : "IS NOT" );
    printf("s1 %s a subset of s4\n", subset(s4,s1) ? "IS" : "IS NOT" );
    printf("s3 %s a subset of s1\n", subset(s1,s3) ? "IS" : "IS NOT" );
    printf("s3 %s a subset of s4\n", subset(s4,s3) ? "IS" : "IS NOT" );
    printf("s4 %s a subset of s1\n", subset(s1,s4) ? "IS" : "IS NOT" );
    printf("s4 %s a subset of s3\n", subset(s3,s4) ? "IS" : "IS NOT" );

    printf("\nAdding 18 to s1:\n");
    ADD(s1, 18);
    printf("s1 %s a subset of s1\n", subset(s1,s1) ? "IS" : "IS NOT" );
    printf("s3 %s a subset of s3\n", subset(s3,s3) ? "IS" : "IS NOT" );
    printf("s1 %s a subset of s3\n", subset(s3,s1) ? "IS" : "IS NOT" );
    printf("s3 %s a subset of s1\n", subset(s1,s3) ? "IS" : "IS NOT" );

    ASSIGN(s2,s3); puts("\ns3       =");
    pset(s2, (pset_t) fprintf, stdout );
    ASSIGN(s2,s3); UNION(s2,s1); puts("\ns1 UNI s3=");
    pset(s2, (pset_t) fprintf, stdout );
    ASSIGN(s2,s3); INTERSECT(s2,s1); puts("\ns1 INT s3=");
    pset(s2, (pset_t) fprintf, stdout );
    ASSIGN(s2,s3); DIFFERENCE(s2,s1); puts("\ns1 DIF s3=");
    pset(s2, (pset_t) fprintf, stdout );


    truncate( s2 );
    printf("s2 has%s been emptied\n", IS_EMPTY(s2) ? "" : " NOT" );

    invert( s2 ); printf("\ns2 inverted = "); pset(s2,(pset_t) fprintf,stdout);
    CLEAR ( s2 ); printf("\ns2 cleared  = "); pset(s2,(pset_t) fprintf,stdout);
    FILL  ( s2 ); printf("\ns2 filled   = "); pset(s2,(pset_t) fprintf,stdout);

    printf("\ns1="); pset( s1, (pset_t) fprintf, stdout );
    printf("\ns3="); pset( s3, (pset_t) fprintf, stdout );
    printf("\ns4="); pset( s4, (pset_t) fprintf, stdout );
    printf("\n");

    /* -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - */

    for( i = 40 ; --i >= 0; )
    {
	a[i] = newset();
	ADD( a[i], i % 20 );
    }

    ADD( a[0],  418 ); REMOVE( a[0],  418 );
    ADD( a[10], 418 ); REMOVE( a[10], 418 );

    printf("\nUnsorted:\n");
    for( i = 0; i < 40; i++ )
    {
	printf( "Set %d: ", i );
	pset(a[i], (pset_t) fprintf, stdout );
	printf( i & 1 ? "\n" : "\t" );
    }

    ssort( a, 40, sizeof(a[0]), scmp );

    printf("\nSorted:\n");
    for( i = 0; i < 40; i++ )
    {
	printf("Set %d: ", i );
	pset(a[i], (pset_t) fprintf, stdout );
	printf( i & 1 ? "\n" : "\t" );
    }
}
#endif
