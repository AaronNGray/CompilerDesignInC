/*@A (C) 1992 Allen I. Holub                                                */



#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/hash.h>
#include <tools/compiler.h>

   /*	HASH.C	General-purpose hash table functions.
    */


PUBLIC void *newsym( size )
int	size;
{
    /* Allocate space for a new symbol; return a pointer to the user space. */

    BUCKET	*sym;

    if( !(sym = (BUCKET *) calloc( size + sizeof(BUCKET), 1)) )
    {
	fprintf( stderr, "Can't get memory for BUCKET\n" );
	raise( SIGABRT );
	return NULL;
    }
    return (void *)( sym + 1 );		/* return pointer to user space */
}

/*----------------------------------------------------------------------*/

PUBLIC void freesym( sym )
void	*sym;
{
    free( (BUCKET *)sym - 1 );
}


PUBLIC HASH_TAB	*maketab( maxsym, hash_function, cmp_function )
unsigned 	maxsym;
unsigned	(*hash_function)();
int		(*cmp_function)();
{
    /*	Make a hash table of the indicated size.  */

    HASH_TAB  *p;

    if( !maxsym )
	maxsym = 127;
    			  /*   |<--- space for table ---->|<- and header -->| */
    if( p=(HASH_TAB*) calloc(1,(maxsym * sizeof(BUCKET*)) + sizeof(HASH_TAB)) )
    {
	p->size    = maxsym	   ;
	p->numsyms = 0		   ;
ANSI(	p->hash    = (unsigned (*)(void*)	) hash_function ; )
UNIX(	p->hash    = 				  hash_function ; )
ANSI(	p->cmp     = (int      (*)(void*,void*) ) cmp_function  ; )
UNIX(	p->cmp     =				  cmp_function  ; )
    }
    else
    {
	fprintf(stderr, "Insufficient memory for symbol table\n");
	raise( SIGABRT );
	return NULL;
    }
    return p;
}


PUBLIC void *addsym( tabp, isym )
HASH_TAB *tabp;
void	 *isym;
{
    /* Add a symbol to the hash table.  */

    BUCKET   **p, *tmp ;
    BUCKET   *sym = (BUCKET *)isym;

    p = & (tabp->table)[ (*tabp->hash)( sym-- ) % tabp->size ];

    tmp	      = *p  ;
    *p	      = sym ;
    sym->prev = p   ;
    sym->next = tmp ;

    if( tmp )
	tmp->prev = &sym->next ;

    tabp->numsyms++;
    return (void *)(sym + 1);
}


PUBLIC	void	delsym( tabp, isym )
HASH_TAB *tabp;
void	 *isym;
{
    /*	Remove a symbol from the hash table. "sym" is a pointer returned from
     *  a previous findsym() call. It points initially at the user space, but
     *  is decremented to get at the BUCKET header.
     */

    BUCKET   *sym = (BUCKET *)isym;

    if( tabp && sym )
    {
	--tabp->numsyms;
	--sym;

	if(  *(sym->prev) = sym->next  )
	    sym->next->prev = sym->prev ;
    }
}



PUBLIC void 	*findsym( tabp, sym )
HASH_TAB	*tabp;
void		*sym;
{
    /*	Return a pointer to the hash table element having a particular name
     *	or NULL if the name isn't in the table.
     */

    BUCKET	*p ;

    if( !tabp )		/* Table empty */
	return NULL;

    p = (tabp->table)[ (*tabp->hash)(sym) % tabp->size ];

    while(  p  &&  (*tabp->cmp)( sym, p+1 )  )
	    p = p->next;

    return (void *)( p ? p + 1 : NULL );
}

/*----------------------------------------------------------------------*/

PUBLIC void    *nextsym( tabp, i_last )
HASH_TAB *tabp;
void	 *i_last;
{
    /* Return a pointer the next node in the current chain that has the same
     * key as the last node found (or NULL if there is no such node). "last"
     * is a pointer returned from a previous findsym() of nextsym() call.
     */

    BUCKET *last = (BUCKET *)i_last;

    for(--last; last->next ; last = last->next )
	    if( (tabp->cmp)(last+1, last->next +1) == 0 )   /* keys match */
		    return (char *)( last->next + 1 );
    return NULL;
}


PRIVATE int (*User_cmp)(void*, void*);

PUBLIC int ptab( tabp, print, param, sort )
HASH_TAB *tabp;		/* Pointer to the table	      	      */
ptab_t	 print;		/* Print function used for output     */
void	 *param;	/* Parameter passed to print function */
int	 sort;		/* Sort the table if true.	      */
{
    /* Return 0 if a sorted table can't be printed because of insufficient
     * memory, else return 1 if the table was printed. The print function
     * is called with two arguments:
     *		(*print)( sym, param )
     *
     * Sym is a pointer to a BUCKET user area and param is the third
     * argument to ptab.
     */

    BUCKET	**outtab, **outp, *sym, **symtab ;
    int		internal_cmp();
    int		i;

    if( !tabp  ||  tabp->size == 0 )	/* Table is empty */
	return 1;

    if( !sort )
    {
	for( symtab = tabp->table, i = tabp->size ;  --i >= 0 ;  symtab++ )
	{
	    /* Print all symbols in the current chain. The +1 in the print call
	     * increments the pointer to the applications area of the bucket.
	     */
	    for( sym = *symtab ; sym ; sym = sym->next )
		(*print)( sym+1, param );
	}
    }
    else
    {
	/*	Allocate memory for the outtab, an array of pointers to
	 *	BUCKETs, and initialize it. The outtab is different from
	 *	the actual hash table in that every outtab element points
	 *	to a single BUCKET structure, rather than to a linked list
	 *	of them.
	 */

	if( !( outtab = (BUCKET **) malloc(tabp->numsyms * sizeof(BUCKET*)) ))
	    return 0;

	outp = outtab;

	for( symtab = tabp->table, i = tabp->size ;  --i >= 0 ;  symtab++ )
	    for( sym = *symtab ; sym ; sym = sym->next )
	    {
		if( outp > outtab + tabp->numsyms )
		{
		    fprintf(stderr,"Internal error [ptab], table overflow\n");
		    exit(1);
		}

		*outp++ = sym;
	    }

	/*	Sort the outtab and then print it. The (*outp)+1 in the
	 *	print call increments the pointer past the header part
	 *	of the BUCKET structure. During sorting, the increment
	 *	is done in internal_cmp.
	 */

	User_cmp = tabp->cmp;
	assort( (void **)outtab, tabp->numsyms, sizeof( BUCKET* ),
				    (int (*)(void*,void*)) internal_cmp );

	for( outp = outtab, i = tabp->numsyms; --i >= 0 ;  outp++ )
	    (*print)( (*outp)+1, param );

	free( outtab );
    }
    return 1;
}

PRIVATE int internal_cmp( p1, p2 )
BUCKET **p1, **p2;
{
    return (*User_cmp)( (void*)(*p1 + 1), (void*)(*p2 + 1) );
}

#ifdef  MAIN
#define MAXLEN	128			/* Used by pstat(), max number	*/
					/* of expected chain lengths. 	*/
/*----------------------------------------------------------------------
 * The following test routines exercise the hash functions by building a table
 * consisting of either 500 words comprised of random letters (if RANDOM is
 * defined) or by collecting keywords and variable names from standard input.
 * Statistics are then printed showing the execution time, the various collision
 * chain lengths and the mean chain length (and deviation from the mean). If
 * you're using real words, the usage is:
 *		cat file ... | hash  	or 	hash < file
 */

#include <time.h>
#include <sys/types.h>
#include <sys/timeb.h>

typedef struct
{
    char     name[32];	/* hash key			   */
    char     str[16];	/* Used for error checking 	   */
    unsigned hval;	/* Hash value of name, also "	   */
    int	  count;	/* # of times word was encountered */
}
STAB;

void		pstats( tabp )
HASH_TAB	*tabp;
{
    /*	Print out various statistics showing the lengths of the
     *	chains (number of collisions)  along with the mean depth
     *	of non-empty chains, standard deviation, etc.
     */

    BUCKET	*p;			/* Pointer to current hash element  */
    int		i;			/* counter			    */
    int		chain_len;		/* length of current collision chain */
    int		maxlen   = 0;		/* maximum chain length		    */
    int		minlen   = MAXINT;	/* minimum chain length		    */
    int		lengths[ MAXLEN ];	/* indexed by chain length, holds   */
					/* the # of chains of that length.  */
    int	longer = 0 ;		/* # of chains longer than MAXLEN   */
    double	m,d, mean();	/* Mean chain length and standard   */
				/* deviation from same		    */

    if( tabp->numsyms == 0 )
    {
	printf("Table is empty\n");
	return;
    }

    mean(1, 0.0, &d);

    memset( lengths, 0, sizeof(lengths) );

    for( i = 0; i < tabp->size ; i++ )
    {
	chain_len = 0;
	for( p = tabp->table[i] ; p ; p = p->next )
	    chain_len++;

	if( chain_len >= MAXLEN )
	    ++longer;
	else
	    ++lengths[chain_len];

	minlen = min( minlen, chain_len );
	maxlen = max( maxlen, chain_len );

	if( chain_len != 0 )
	    m = mean( 0, (double)chain_len, &d );
    }

    printf("%d entries in %d element hash table, ", tabp->numsyms, tabp->size );
    printf("%d (%1.0f%%) empty.\n",
		 lengths[0], ((double)lengths[0]/tabp->size) * 100.00);

    printf("Mean chain length (excluding zero-length chains): %g\n", m);
    printf("\t\t\tmax=%d, min=%d, standard deviation=%g\n", maxlen, minlen, d );

    for( i = 0; i < MAXLEN; i++ )
	if( lengths[i] )
		printf("%3d chains of length %d\n", lengths[i], i );

    if( longer )
	printf("%3d chains of length %d or longer\n", longer, MAXLEN );
}

/*----------------------------------------------------------------------*/

dptab( addr )
HASH_TAB	*addr;
{
	/*!!!!!!!!!!!!!!!!!!!!!! WARNING !!!!!!!!!!!!!!!!!!!!!!!!!
	 *  I'm using %x to print pointers. Change this
	 *  to %p in the compact or large models.
	 */

	BUCKET	**p, *bukp ;
	int	i;

	printf("HASH_TAB at 0x%04x (%d element table, %d symbols)\n",
				addr, addr->size, addr->numsyms );

	for( p = addr->table, i = 0 ; i < addr->size ; ++p, ++i )
	{
	    if( !*p )
		    continue;

	    printf("Htab[%3d] @ %04x:\n", i, p );

	    for( bukp = *p; bukp ; bukp=bukp->next )
	    {
		printf("\t\t%04x prev=%04x next=%04x user=%04x",
		     bukp, bukp->prev, bukp->next, bukp+1);

		printf(" (%s)\n", ((STAB *)(bukp+1))->name );
	    }

	    putchar('\r');
	}
}

/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

getword( buf )
char	*buf;
{

#ifdef RANDOM	/* ----------------- Generate 500 random words -------------*/

	static int	wordnum = 500;
	int		num_letters, let;

	if( --wordnum < 0 )
	    return 0;

	while( (num_letters = rand() % 16)  < 3  )
	    ;

	while( --num_letters >= 0  )
	{
	    let    = (rand() % 26) + 'a' ;	/* 26 letters in english */
	    *buf++ = (rand() % 10) ? let : toupper(let) ;
	}

#else	/* ------------------------ Get words from standard input ----------*/

	int	c;		/* Current character.			*/

	while( (c = getchar()) != EOF )
	{
	    /* Skip up to the beginning of the next word, ignoring the
	     * contents of all comments.
	     */

	    if( isalpha(c) || c=='_')
		break;

	    if( c == '/' )
	    {
		if( (c = getchar()) != '*' )
		{
		    if( c == EOF )
			break;

		    ungetc(c, stdin);
		    continue;
		}
		else
		{
		    /* Absorb a comment */

		    while( (c = getchar()) != EOF )
			if( c == '*' && getchar() == '/' )
			    break;
		}
	    }
	}

	if( c == EOF )
		return 0;
	else
	{
	    /* Collect the word. Note that numbers (words with no
	     * (letters in them or hex constants) are ignored.
	     */

	    *buf++ = c;
	    while( (c = getchar()) != EOF  &&  (isalnum(c) || c=='_') )
		*buf++ = c;
	}
#endif

	*buf = '\0';
	return 1;
}
/*- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
main( argc, argv )
char	**argv;
{
    char	 word[ 80 ];
    STAB	 *sp;
    HASH_TAB	 *tabp;
    struct timeb start_time, end_time ;
    double	 time1, time2;
    int		 c;

    /* hash a <list 	for hash_add */
    /* hash p <list 	for hash_pjw */

    c = (argc > 1) ? argv[1][0] : 'a' ;

    printf( "\nUsing %s\n", c=='a' ? "hash_add" : "hash_pjw" );

    tabp = maketab( 127, c=='a' ? hash_add : hash_pjw , strcmp );

    ftime( &start_time );

    while( getword( word ) )
    {
	if( sp = (STAB *) findsym(tabp, word) )
	{
	    if( strcmp(sp->str,"123456789abcdef") ||
					( sp->hval != hash_add( word )) )
	    {
		printf("NODE HAS BEEN ADULTERATED\n");
		exit( 1 );
	    }

	    sp->count++;
	}
	else
	{
            sp = newsym( sizeof(STAB) );
	    strncpy( sp->name, word, 32 );
	    strcpy ( sp->str, "123456789abcdef" );
	    sp->hval = hash_add( word );
	    addsym( tabp, sp );
	    sp->count = 1;
	}
    }

    ftime( &end_time );
    time1 = (start_time.time * 1000) + start_time.millitm ;
    time2 = (  end_time.time * 1000) +   end_time.millitm ;
    printf( "Elapsed time = %g seconds\n", (time2-time1) / 1000 );

    pstats( tabp );

    /* dptab( tabp ); */
}
#endif
