/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __VBIOS_H
#define __VBIOS_H

#include <tools/termlib.h>

#define VIDEO_INT     0x10    /* Video interrupt	    */
#define KB_INT	      0x16    /* Keyboard interrupt	    */
#define CUR_SIZE      0x1     /* Set cursor size	    */
#define SET_POSN      0x2     /* Modify cursor posn	    */
#define READ_POSN     0x3     /* Read current cursor posn   */
#define	SCROLL_UP     0x6     /* scroll region of screen up */
#define	SCROLL_DOWN   0x7     /*  " down		    */
#define READ_CHAR     0x8     /* Read character from screen */
#define WRITE	      0x9     /* Write character	    */
#define WRITE_TTY     0xe     /* Write char & move cursor   */
#define GET_VMODE     0xf     /* Get video mode & disp pg   */
/* These video-BIOS functions are implemented as macros. The ones marked with
 * stars have side effects.
 *
 * VB_INCHA  	Returns the character and attribute ORed together.  Character in
 *		the low byte and attribute in the high byte.
 * VB_GETPAGE 	Return the currently active display page number
 * VB_GETCUR 	Get current cursor position. The top byte of the return value
 *		holds the row, the bottom by the column. Pagenum is the video
 *		page number. Note that VB_GETPAGE() will mess up the fields in
 *		the Regs structure so it must be called first.
 * VB_CURSIZE	Change the cursor shape to go from the top to the bottom scan
 *		line.
 * VB_OUTCHA  *	Write a character and attribute without moving the cursor. The
 *		attribute is in c's high byte, the character is the low byte.
 * VB_REPLACE *	Same as VB_OUTCHA but uses the existing attribute byte.
 * VB_SETCUR	Modify current cursor position. The top byte of "posn" value
 *		holds the row (y), the bottom byte, the column (x). The top-left
 *		corner of the screen is (0,0). Pagenum is the video-display-page
 *		number.
 * VB_CTOYX(y,x)   Like VB_SETCUR but y and x coordinates are used.
 * VB_SCROLL	 * Scroll the indicated region on the screen. If amt is <0,
 *		   scroll down; otherwise, scroll up.
 * VB_CLRS	   Clear the entire screen
 * VB_CLR_REGION * Clear a region of the screen
 * VB_BLOCKCUR     Change to a block cursor.
 * VB_NORMALCUR    Change to an underline cursor.
 * VB_PUTCHAR	   like vb_putc, but uses white on black for the attribute.
 */

#define	VB_GETPAGE()	_Vbios( GET_VMODE, 0, 0, 0, 0,			  "bh")
#define	VB_INCHA()	_Vbios( READ_CHAR, 0, VB_GETPAGE(), 0, 0,         "ax")
#define	VB_GETCUR()	_Vbios( READ_POSN, 0, VB_GETPAGE(), 0, 0,	  "dx")
#define	VB_CURSIZE(t,b) _Vbios( CUR_SIZE,  0,0,((t)<<8)|(b),0,            "ax")
#define	VB_OUTCHA(c)	_Vbios( WRITE,     (c)&0xff,((c)>>8)&0xff, 1, 0,  "ax")
#define	VB_REPLACE(c)	VB_OUTCHA( (c & 0xff) | (VB_INCHA() & ~0xff) )
#define	VB_SETCUR(posn) _Vbios( SET_POSN,  0, VB_GETPAGE() << 8, 0,(posn), "ax")
#define	VB_CTOYX(y,x)	VB_SETCUR( ((y) << 8) | ((x) & 0xff) )

#define VB_SCROLL(xl, xr, yt, yb, amt, attr) _Vbios( \
				((amt) < 0) ? SCROLL_DOWN : SCROLL_UP, \
				abs(amt), (attr) << 8, ((yt) << 8) | (xl), \
						       ((yb) << 8) | (xr), "ax"\
						   )

#define VB_CLRS(at)		  VB_SCROLL( 0,  79,  0, 24, 25,         (at))
#define VB_CLR_REGION(l,r,t,b,at) VB_SCROLL( (l),(r),(t),(b),((b)-(t))+1,(at))

#define VB_BLOCKCUR()   VB_CURSIZE( 0, vb_iscolor() ? 7 : 12 )
#define VB_NORMALCUR()  ( vb_iscolor() ? VB_CURSIZE(6,7) : VB_CURSIZE(11,12) )
#define VB_PUTCHAR(c)   vb_putc( (c), NORMAL )

#endif /* __VBIOS_H */
