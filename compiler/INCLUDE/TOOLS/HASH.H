/*@A (C) 1992 Allen I. Holub                                                */
   /* HASH.H	Header required by the hash functions in /src/tools/hash.c */
#ifndef __HASH_H
#define __HASH_H
#include <tools/debug.h>

typedef struct BUCKET
{
    struct BUCKET	  *next;
    struct BUCKET	 **prev;

} BUCKET;


typedef struct  hash_tab_
{
    int	     size     ;		/* Max number of elements in table	 */
    int	     numsyms  ;		/* number of elements currently in table */
    unsigned (*hash) P((void*));       /* hash function		 	 */
    int	     (*cmp ) P((void*,void*)); /* comparison funct, cmp(name,bucket_p); */
    BUCKET   *table[1];		/* First element of actual hash table	 */

} HASH_TAB;

typedef void( *ptab_t )(void *, ... );	/* print argument to ptab */

extern HASH_TAB *maketab P(( unsigned maxsym, unsigned (*hash)(), int(*cmp)()));
extern void    *newsym  P(( int size  ));
extern void    freesym  P(( void *sym ));
extern void    *addsym  P(( HASH_TAB *tabp, void *sym  ));
extern void    *findsym P(( HASH_TAB *tabp, void *sym  ));
extern void    *nextsym P(( HASH_TAB *tabp, void *last ));
extern void    delsym   P(( HASH_TAB *tabp, void *sym  ));
extern int     ptab     P(( HASH_TAB *tabp, ptab_t print, void *par, int srt));
unsigned hash_add	P(( unsigned char *name ));	/* in hashadd.c */
unsigned hash_pjw	P(( unsigned char *name ));	/* in hashpjw.c */

#endif /* __HASH_H */
