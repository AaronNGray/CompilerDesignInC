/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/termlib.h>
#include <tools/vbios.h>

/* GLUE.C	This file glues the curses package to either the video-
 *		BIOS functions or to the direct-video functions.
 */

#if (!defined(R) && !defined(V))
#	define AUTOSELECT
#endif

#pragma comment(exestr,"IBM-CURSES (C)" __DATE__ "Allen I. Holub. All rights reserved.")

/*----------------------------------------------------------------------*/
#ifdef R
#pragma message( "/* Compiling for ROM-BIOS access */" )

init(how)		   { return 0;					}
cmove(y,x)		   { return VB_CTOYX   (y,x);			}
curpos(yp,xp) int *yp,*xp; { return vb_getyx   (yp,xp);			}
replace(c)		   { return VB_REPLACE (c);			}
doscroll(l,r,t,b,a,at)	   { return VB_SCROLL  (l,r,t,b,a, at );	}
inchar()	     	   { return VB_INCHA   ( )  & 0xff ;		}
incha() 		   { return VB_INCHA   ( );			}
outc(c, attrib)		   { return vb_putc    (c, attrib);		}
SBUF *savescr(l,r,t,b)	   { return vb_save    (l,r,t,b);		}
SBUF *restore(b)  SBUF *b; { return vb_restore (b);			}
freescr(p)        SBUF *p; { return vb_freesbuf(p);			}
clr_region(l,r,t,b,attrib) { return VB_CLR_REGION(l,r,t,b,attrib);	}

int is_direct(){ return 0; };
#endif

/*----------------------------------------------------------------------*/
#ifdef V
#pragma message( "/* Compiling for direct-video access */" )

cmove(y,x)		   { return dv_ctoyx   (y,x);			}
curpos(yp,xp) int *yp,*xp; { return dv_getyx   (yp,xp);			}
replace(c)		   { return dv_replace (c);			}
doscroll(l,r,t,b,a,at)	   { return dv_scroll  (l,r,t,b,a,at);		}
inchar()	     	   { return dv_incha   ( ) & 0xff;		}
incha()			   { return dv_incha   ( );			}
outc(c,attrib)		   { return dv_putc    (c, attrib);		}
SBUF *savescr(l,r,t,b)	   { return dv_save    (l,r,t,b);		}
SBUF *restore(b)  SBUF *b; { return dv_restore (b);			}
freescr(p)  	  SBUF *p; { return dv_freesbuf(p);			}
clr_region(l,r,t,b,attrib) { return dv_clr_region(l,r,t,b,attrib);	}

init( how )						/* Initialize */
{
    if( !dv_init() )
    {
	fprintf(stderr, "MGA or CGA in 80-column text mode required\n");
	exit( 1 );
    }
    return 1;
}

int is_direct(){ return 1; };
#endif

/*----------------------------------------------------------------------*/
#ifdef A
#pragma message( "/* Compiling for autoselect */" )

static int Dv = 0 ;

init( how )
int how;	/* 0=BIOS, 1=direct video, -1=autoselect */
{
    char *p;

    if( Dv = how )
    {
	if( how < 0 )
	{
	    p  = getenv( "VIDEO" );
	    Dv = p && ((strcmp(p,"DIRECT")==0 || strcmp(p,"direct")==0));
	}

	if( Dv && !dv_init() )
	{
	    fprintf(stderr, "MGA or CGA in 80-column text mode required\n");
	    exit( 1 );
	}
    }

    return Dv;
}

/* The following statements all depend on the fact that a subroutine name
 * (without the trailing argument list and parentheses) evaluates to a temporary
 * variable of type pointer-to-function. So function names can be treated
 * as a pointer to a function in the conditionals, below. For example, curpos()
 * calls dv_getyx if Dv is true, otherwise vb_getyx is called. ANSI actually
 * says that the star isn't necessary. The following (without the stars in
 * front of the function names) should be legal:
 *
 *	return (Dv ? dv_ctyx : VB_CTOYX)( y, x );
 *
 * but many compilers (including Microsoft) don't accept it. Macros expansions
 * (as compared to subroutine calls) clearly must be treated in a more
 * standard, though less efficient, fashion.
 */

void curpos(yp,xp)int *yp,*xp; { (Dv? *dv_getyx  :*vb_getyx   )(yp, xp);    }

SBUF *savescr(l,r,t,b)	  {return(Dv? *dv_save    :*vb_save    )(l,r,t,b);   }
SBUF *restore(b) SBUF *b; {return(Dv? *dv_restore :*vb_restore )(b);	     }
void freescr(p)  SBUF *p; {	 (Dv? *dv_freesbuf:*vb_freesbuf)(p);	     }
void outc(c, attrib) 	  {	 (Dv? *dv_putc    :*vb_putc    )(c, attrib); }

void replace(c)		  { if(Dv) dv_replace(c); else VB_REPLACE(c);	      }
void cmove(y,x)		  { if(Dv) dv_ctoyx(y,x); else VB_CTOYX(y,x);	      }
inchar()	     	  { return( Dv? dv_incha()  : VB_INCHA()    ) & 0xff; }
incha() 		  { return( Dv? dv_incha()  : VB_INCHA()    );        }

void doscroll(l,r,t,b,a,at)
{
    if( Dv ) dv_scroll(l,r,t,b,a,at);
    else     VB_SCROLL(l,r,t,b,a,at);
}

void clr_region(l,r,t,b,attrib)
{
    if( Dv ) dv_clr_region(l,r,t,b,attrib);
    else     VB_CLR_REGION(l,r,t,b,attrib);
}

int is_direct(){ return Dv; };
#endif
