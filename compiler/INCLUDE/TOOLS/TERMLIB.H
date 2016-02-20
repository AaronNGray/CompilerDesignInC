/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __TERMLIB_H
#define __TERMLIB_H

#include <tools/debug.h>

/* Various definitions for the termlib. Note that if your program includes both
 * termlib.h and vbios.h, termlib.h must be included FIRST.
 */

#define BLACK		0x00	/* Color Card.	   */
#define BLUE		0x01
#define GREEN		0x02
#define CYAN		0x03
#define RED		0x04
#define MAGENTA		0x05
#define BROWN		0x06
#define WHITE		0x07

#define FGND(color)	 (color)
#define BGND(color)	((color) <<4)

#define	NORMAL		(FGND(WHITE) | BGND(BLACK))   /* Monochrome card */
#define UNDERLINED	(FGND(BLUE)  | BGND(BLACK))
#define REVERSE 	(FGND(BLACK) | BGND(WHITE))

#define BLINKING   	0x80	/* May be ORed with the above	*/
#define BOLD    	0x08	/* and with each other		*/

/*----------------------------------------------------------------------
 * If USE_FAR_HEAP is true then use the far heap to save screen images in the
 * small model. You must recompile the termlib if you change this #define.
 */

typedef unsigned int WORD;

#if( USE_FAR_HEAP )
	typedef WORD FARPTR IMAGEP;
	#define IMALLOC  _fmalloc
	#define IFREE    _ffree
#else
	typedef WORD     *IMAGEP;
	#define IMALLOC  malloc
	#define IFREE    free
#endif

typedef struct	SBUF /* used by vb_save, vb_restore, dv_save, and dv_restore */
{
	unsigned int  top, bottom, left, right;
	IMAGEP	      image;
} SBUF;


/*----------------------------------------------------------------------
 *	Prototypes for the video-BIOS access routines.
 */

extern  int  vb_iscolor	      ( void 					  );
extern  void vb_getyx	      ( int *yp,int *xp 			  );
extern  void vb_putc	      ( int c, int attrib		  	  );
extern  void vb_puts	      ( char *str,int move_cur 		    	  );
extern  int  vb_getchar	      ( void 					  );
extern	SBUF *vb_save	      ( int l,int r,int t,int b 		  );
extern	SBUF *vb_restore      ( SBUF *sbuf 				  );
extern  void vb_freesbuf      ( SBUF *sbuf 				  );

extern int _Vbios (int service, int al, int bx, int cx, int dx,
							 char *return_this );
/*----------------------------------------------------------------------
 *	Prototypes for the equivalent direct video functions.
 */

extern  int  dv_init	      ( void					 );
extern  void dv_scroll_line   ( int x_left,int x_right,int y_top,	 \
					int y_bottom, int dir,int attrib );
extern  void dv_scroll	      ( int x_left,int x_right,int y_top,	 \
					int y_bottom, int amt,int attrib );
extern  void dv_clrs	      ( int attrib 				 );
extern  void dv_clr_region    ( int l,int r,int t,int b,int attrib	 );
extern  void dv_ctoyx	      ( int y,int x				 );
extern  void dv_getyx	      ( int *rowp,int *colp			 );
extern  void dv_putc	      ( int c,int attrib			 );
extern  void dv_putchar	      ( int c					 );
extern  void dv_puts	      ( char *str,int move_cur		 	 );
extern  void dv_putsa	      ( char *str,int attrib			 );
extern  int  dv_incha	      ( void					 );
extern  void dv_outcha	      ( int c					 );
extern  void dv_replace	      ( int c					 );
extern  void dv_printf	      ( int attribute,char *fmt 	VA_LIST	 );
extern  SBUF *dv_save	      ( int l,int r,int t,int b		 	 );
extern  SBUF *dv_restore      ( SBUF *sbuf				 );
extern  void dv_freesbuf      ( SBUF *sbuf				 );

#endif /* __TERMLIB_H */
