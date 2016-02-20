/*@A (C) 1992 Allen I. Holub                                                */
#line 111 "0.tr"
#ifndef __DEBUG_H	/* Makes sure that debug.h isn't included more than */
#define __DEBUG_H	/* once. Matching endif is at end of file.	    */

#ifdef  DEBUG
#	define PRIVATE
#	define D(x) x   /* expand only when debugging     */
#	define ND(x)    /* expand only when not debugging */
#else
#	define PRIVATE static
#	define D(x)
#	define ND(x) x
#endif
#define PUBLIC

#ifdef __TURBOC__	/* Compiling for Turbo/Borland C/C++	*/
#    define BCC(x)  x	/* BCC(x) expands to its argument	*/
#    define MSC(x)	/* these expand to empty strings	*/
#    define MSC5(x)
#    define MSC6(x)
#    define MSC7(x)
#    define UNIX(x)
#    define ANSI(x) x
#    define _8086
#    define FARPTR far*
#    define VA_LIST
#    define MSDOS
#else
#if (defined(MSDOS) || defined(_MSDOS))
#    ifndef MSDOS   		// In case the MSDOS predefinition
#        define MSDOS 1		// disappears in future versions.
#    endif
#    if( defined(_MSC_VER) && _MSC_VER==700 )
#       pragma message( "/* Compiling for Microsoft C, Ver. 7.x */" )
#	define FARPTR __far*
#	define MSC5(x)
#	define MSC6(x)
#       define MSC7(x) x
#    elif( defined(MSC_VER) && MSC_VER==600 )
#       pragma message( "/* Compiling for Microsoft C, Ver. 6.x */" )
#	define FARPTR *far
#	define MSC5(x)
#	define MSC6(x) x
#       define MSC7(x)
#    else
#       pragma message( "/* Compiling for Microsoft C, Ver. 5.x */" )
#	define FARPTR far*
#	define MSC5(x) x
#	define MSC6(x)
#       define MSC7(x)
#    endif
#	define BCC(x)		/* All versions */
#	define MSC(x) x
#	define UNIX(x)
#	define ANSI(x) x
#	define _8086
#	define VA_LIST  ,...
#else				/* non-ansi (ie. UNIX) compiler */
#	define BCC(x)
#	define MSC(x)
#	define MSC5(x)
#	define MSC6(x)
#       define MSC7(x)
#	define UNIX(x) x
#	define ANSI(x)
#	define FARPTR *
#	define VA_LIST

#	define O_BINARY 0	/* No binary input mode in UNIX open().     */
#	define far		/* Microsoft/Turbo keyword for an 8086      */
				/* 32-bit, far pointer. Ignore in UNIX.     */
#	define const		/* Ignore ANSI const and volatile keywords. */
#	define volatile
#	define memmove(d,s,n) bcopy(s,d,n)
#	define memcpy(d,s,n)  bcopy(s,d,n)
	extern long getpid();
#	define raise(sig) kill( (int)getpid(), sig )
#	define vfprintf(stream, fmt, argp) _doprnt( fmt, argp, stream )
#	define vprintf (	fmt, argp) _doprnt( fmt, argp, stdout )

	typedef long time_t;	 /* for the VAX, may have to change this */
	typedef unsigned size_t; /* for the VAX, may have to change this */
	extern char *strdup();	 /* You need to supply one.		*/
	typedef int void;
#endif /*  MSDOS   */
#endif /*__TURBOC__*/

#if (0 ANSI(+1))
#	define KnR(x)
#	define P(x)  x	/* function prototypes supported */
#else
#	define KnR(x)  x
#	define P(x)  ()	  /* Otherwise, discard argument lists and */
#	define void  char /* translate void keyword to int.	   */
#endif

#if ( 0 MSC(+1) BCC(+1) ) 	/* Microsoft or Borland Compiler */
#	define MS(x) x
#else
#	define MS(x)
#endif

/* SEG(p)	Evaluates to the segment portion of an 8086 address.
 * OFF(p)	Evaluates to the offset portion of an 8086 address.
 * PHYS(p)	Evaluates to a long holding a physical address
 */

#ifdef _8086
#define SEG(p)  ( ((unsigned *)&(p))[1] )
#define OFF(p)  ( ((unsigned *)&(p))[0] )
#define PHYS(p) (((unsigned long)OFF(p)) + ((unsigned long)SEG(p) << 4))
#else
#define PHYS(p)  (p)
#endif

/* NUMELE(array)	Evaluates to the array size in elements
 * LASTELE(array)	Evaluates to a pointer to the last element
 * INBOUNDS(array,p)    Evaluates to true if p points into the array.
 * RANGE(a,b,c)		Evaluates to true if a <= b <= c
 * max(a,b)		Evaluates to a or b, whichever is larger
 * min(a,b)		Evaluates to a or b, whichever is smaller
 *			associated with a pointer
 * NBITS(type)		Returns number of bits in a variable of the indicated
 *			type;
 * MAXINT		Evaluates to the value of the largest signed integer
 */

#define NUMELE(a)	(sizeof(a)/sizeof(*(a)))
#define LASTELE(a)	((a) + (NUMELE(a)-1))
#define TOOHIGH(a,p)	((p) - (a) > (NUMELE(a) - 1))
#define TOOLOW(a,p)	((p) - (a) <  0 )
#define INBOUNDS(a,p)	( ! (TOOHIGH(a,p) || TOOLOW(a,p)) )

/* Portability note: Some systems won't allow UL for unsined long in the _IS
 * macro. You can use the following if so:
 *
 *	(unsigned long)1
 *
 * Bob Muller, who suggested the foregoing, also reports:
 * "There also seems to be an issue with the notion of shifting; the DEC Ultrix
 * compiler, for example, says that (unsigned long)((unsigned long)1 << 32)
 * == 1, while the Sun SunOS 4 compiler says that it is 0."
 */

#define _IS(t,x) ( ((t)(1UL << (x))) !=0)
				      /* Evaluate true if the width of a      */
				      /* variable of type of t is < x. The != */
				      /* 0 assures that the answer is 1 or 0  */

#define NBITS(t) (4 * (1  + _IS(t, 4) + _IS(t, 8) + _IS(t,12) + _IS(t,16) \
		          + _IS(t,20) + _IS(t,24) + _IS(t,28) + _IS(t,32) ) )

#define MAXINT (((unsigned)~0) >> 1)

#if ( 0 UNIX(+1) )
#    ifndef max
#        define max(a,b) ( ((a) > (b)) ? (a) : (b))
#    endif
#    ifndef min
#        define min(a,b) ( ((a) < (b)) ? (a) : (b))
#    endif
#endif

#define RANGE(a,b,c)	( (a) <= (b) && (b) <= (c) )

   /* The distribution disk doesn't include dmalloc.h, so don't define
    * MAP_MALLOC anywhere.
    */
#ifdef MAP_MALLOC
#    include <tools/dmalloc.h>
#endif

#endif	/* #ifdef __DEBUG_H */


