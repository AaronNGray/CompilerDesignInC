/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <tools/debug.h>	/* For ANSI() and KnR() definitions.	*/
#undef SEG			/* Redefined in c-code.h.		*/
#undef P			/* Redefined below.			*/
#include <tools/c-code.h>

#ifdef __TURBOC__	/* If Borland/Turbo C, suppress warning caused   */
#pragma warn -rch	/* by unreachable code (mostly _T() invocations) */
#endif	 	/* that follow returns.				 */
#ifdef ALLOC
#    define I(x)  x
#    define CLASS /* empty */
#else
#    define I(x)  /* empty */
#    define CLASS extern
#endif
					/* Basic types		*/
typedef char  byte;			/* 8  bit 		*/
typedef short word;			/* 16 bit 		*/
typedef long  lword;			/* 32 bit 		*/
typedef char  *ptr;			/* Nominally 32 bit.	*/

typedef byte  array;			/* Aliases for "byte."	*/
typedef byte  record;


struct _words  { word	low, high; 	};
struct _bytes  { byte	b0, b1, b2, b3;	};	/* b0 is LSB, b3 is MSB */

typedef union reg
{
    char	   *pp;  	/* pointer 			*/
    lword   	   l;		/* long word			*/
    struct _words  w;		/* two 16-bit words     	*/
    struct _bytes  b;		/* four 8-bit bytes		*/
}
reg;

CLASS reg r0, r1, r2, r3, r4, r5, r6, r7 ;	/* Registers */
CLASS reg r8, r9, rA, rB, rC, rD, rE, rF ;

CLASS reg	stack[ SDEPTH ];		/* run-time stack */
CLASS reg	*__sp I(= &stack[ SDEPTH ]);	/* Stack pointer  */
CLASS reg	*__fp I(= &stack[ SDEPTH ]);	/* Frame pointer  */

#define fp	((char *) __fp)
#define sp	((char *) __sp)

#define SEG(segment)	/* empty */

#define public		/* empty */
#define common		/* empty */
#define private		static
#define external 	extern

#define	ALIGN(type)  /* empty */

#define W	* (word   *)
#define B	* (byte   *)
#define L	* (lword  *)
#define P	* (ptr    *)
#define WP	* (word  **)
#define BP	* (byte  **)
#define LP	* (lword **)
#define PP	* (ptr   **)

#define push(n)	        (--__sp)->l = (lword)(n)
#define pop(t)		(t)( (__sp++)->l )


#if ( 0 ANSI(+1) )
#define PROC(name,cls)	cls void name ( void ){
#else
#define PROC(name,cls)		cls name () {
#endif
#define ENDP(name)	_:ret();}			/* Name is ignored. */

#if ( 0  ANSI(+1) )
#define call(name)   ((--__sp)->pp = #name,        (*(void (*)(void))(name))())
#else
#define call(name) ((--__sp)->pp = "<ret addr>", (*(void (*)(    ))(name))())
#endif

#define ret()           __sp++; return
#define link(n)	  ((--__sp)->pp = (char *)__fp) , (__fp = __sp) , (__sp -= (n))

#define unlink()  (__sp = (reg *)__fp) , (__fp = (reg *)((__sp++)->pp))
#define lrs(x,n)   ((x) = ((unsigned long)(x) >> (n)))

#define ext_low(reg)	(reg.w.low  = (word )reg.b.b0	)
#define ext_high(reg)	(reg.w.high = (word )reg.b.b2	)
#define ext_word(reg)	(reg.l      = (lword)reg.w.low	)


#define EQ(a,b)		if( (long)(a) == (long)(b) )
#define NE(a,b)		if( (long)(a) != (long)(b) )
#define LT(a,b)		if( (long)(a) <  (long)(b) )
#define LE(a,b)		if( (long)(a) <= (long)(b) )
#define GT(a,b)		if( (long)(a) >  (long)(b) )
#define GE(a,b)		if( (long)(a) >= (long)(b) )

#define U_LT(a,b)	if( (unsigned long)(a) <  (unsigned long)(b) )
#define U_GT(a,b)	if( (unsigned long)(a) >  (unsigned long)(b) )
#define U_LE(a,b)	if( (unsigned long)(a) <= (unsigned long)(b) )
#define U_GE(a,b)	if( (unsigned long)(a) >= (unsigned long)(b) )

#define BIT(b,s)	if( (s) & (1 << (b)) )

#define  _main  main

ANSI( void pm(void); )
#ifdef ALLOC
void pm()
{
    reg *p;
    int i;

    /* Print the virtual machine (registers and top 16 stack elements). */

    printf("r0= %08lx  r1= %08lx  r2= %08lx  r3= %08lx\n",
	    r0.l,      r1.l,	  r2.l,	     r3.l 		);
    printf("r4= %08lx  r5= %08lx  r6= %08lx  r7= %08lx\n",
	    r4.l,      r5.l,	  r6.l,	     r7.l 		);
    printf("r8= %08lx  r9= %08lx  rA= %08lx  rB= %08lx\n",
	    r8.l,      r9.l,	  rA.l,	     rB.l 		);
    printf("rC= %08lx  rD= %08lx  rE= %08lx  rF= %08lx\n",
	    rC.l,      rD.l,	  rE.l,	     rF.l 		);

    if( __sp >= &stack[SDEPTH] )
	printf("Stack is empty\n");
    else
	printf("\nitem byte real addr   b3 b2 b1 b0      hi   lo          l\n");

    for( p = __sp, i=16; p < &stack[SDEPTH]  &&  --i>=0; ++p )
    {
	printf("%04d %04d %9p  [%02x|%02x|%02x|%02x] = [%04x|%04x] = [%08lx]",
	   p-__sp,             (p-__sp)*4,       MSC((void far *))p,
	   p->b.b3 & 0xff,     p->b.b2 & 0xff,   p->b.b1 & 0xff, p->b.b0 & 0xff,
	   p->w.high & 0xffff, p->w.low & 0xffff,
	   p->l
	);

	if( p == __sp ) printf("<-SP");
	if( p == __fp ) printf("<-FP");
	printf("\n");
    }
}
#endif
