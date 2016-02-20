/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __BOX_H
#define __BOX_H
/*-------------------------------------------------------
 * BOX.H:  #defines for the box-drawing characters
 *-------------------------------------------------------
 *	The names are:
 *
 *	UL	Upper left corner
 *	UR	Upper right corner
 *	LL	lower left corner
 *	LR	lower right corner
 *	CEN	Center (intersection of two lines)
 *	TOP	Tee with the flat piece on top
 *	BOT	Bottom tee
 *	LEFT	Left tee
 *	RIGHT	Right tee
 *	HORIZ	Horizontal line
 *	VERT	Vertical line.
 *
 *			UL    -TOP-	UR 		HORIZ
 *				|
 *			L		 R		  V
 *		  	E	|	 I		  E
 *			F--   -CEN-    --G		  R
 *			T	|	 H		  T
 *			|		 T
 *				|
 *			LL    -BOT-	LR
 *
 * The D_XXX  defines have double horizontal and vertical lines.
 * The HD_XXX defines have double horizontal lines and single  vertical  lines.
 * The VD_XXX defines have double  vertical  lines and single horizontal lines.
 *
 * If your terminal is not IBM compatible, #define all of these as '+' (except
 * for the VERT #defines, which should be a |, and the HORIZ #defines, which
 * should be a -) by #defining NOT_IBM_PC before including this file.
 */

#ifdef NOT_IBM_PC
#    define IBM_BOX(x)
#    define OTHER_BOX(x) x
#else
#    define IBM_BOX(x)   x
#    define OTHER_BOX(x)
#endif

#define	VERT		IBM_BOX( 179 )	OTHER_BOX( '|' )
#define	RIGHT		IBM_BOX( 180 )	OTHER_BOX( '+' )
#define	UR		IBM_BOX( 191 )	OTHER_BOX( '+' )
#define	LL		IBM_BOX( 192 )	OTHER_BOX( '+' )
#define	BOT		IBM_BOX( 193 )	OTHER_BOX( '+' )
#define	TOP		IBM_BOX( 194 )	OTHER_BOX( '+' )
#define	LEFT		IBM_BOX( 195 )	OTHER_BOX( '+' )
#define	HORIZ		IBM_BOX( 196 )	OTHER_BOX( '-' )
#define	CEN		IBM_BOX( 197 )	OTHER_BOX( '+' )
#define	LR		IBM_BOX( 217 )	OTHER_BOX( '+' )
#define	UL		IBM_BOX( 218 )	OTHER_BOX( '+' )
#define	D_VERT		IBM_BOX( 186 )	OTHER_BOX( '|' )
#define	D_RIGHT		IBM_BOX( 185 )	OTHER_BOX( '+' )
#define	D_UR		IBM_BOX( 187 )	OTHER_BOX( '+' )
#define	D_LL		IBM_BOX( 200 )	OTHER_BOX( '+' )
#define	D_BOT		IBM_BOX( 202 )	OTHER_BOX( '+' )
#define	D_TOP		IBM_BOX( 203 )	OTHER_BOX( '+' )
#define	D_LEFT		IBM_BOX( 204 )	OTHER_BOX( '+' )
#define	D_HORIZ		IBM_BOX( 205 )	OTHER_BOX( '-' )
#define	D_CEN		IBM_BOX( 206 )	OTHER_BOX( '+' )
#define	D_LR		IBM_BOX( 188 )	OTHER_BOX( '+' )
#define	D_UL		IBM_BOX( 201 )	OTHER_BOX( '+' )
#define	HD_VERT		IBM_BOX( 179 )	OTHER_BOX( '|' )
#define	HD_RIGHT	IBM_BOX( 181 )	OTHER_BOX( '+' )
#define	HD_UR		IBM_BOX( 184 )	OTHER_BOX( '+' )
#define	HD_LL		IBM_BOX( 212 )	OTHER_BOX( '+' )
#define	HD_BOT		IBM_BOX( 207 )	OTHER_BOX( '+' )
#define	HD_TOP		IBM_BOX( 209 )	OTHER_BOX( '+' )
#define	HD_LEFT		IBM_BOX( 198 )	OTHER_BOX( '+' )
#define	HD_HORIZ	IBM_BOX( 205 )	OTHER_BOX( '-' )
#define	HD_CEN		IBM_BOX( 216 )	OTHER_BOX( '+' )
#define	HD_LR		IBM_BOX( 190 )	OTHER_BOX( '+' )
#define	HD_UL		IBM_BOX( 213 )	OTHER_BOX( '+' )
#define	VD_VERT		IBM_BOX( 186 )	OTHER_BOX( '|' )
#define	VD_RIGHT	IBM_BOX( 182 )	OTHER_BOX( '+' )
#define	VD_UR		IBM_BOX( 183 )	OTHER_BOX( '+' )
#define	VD_LL		IBM_BOX( 211 )	OTHER_BOX( '+' )
#define	VD_BOT		IBM_BOX( 208 )	OTHER_BOX( '+' )
#define	VD_TOP		IBM_BOX( 210 )	OTHER_BOX( '+' )
#define	VD_LEFT		IBM_BOX( 199 )	OTHER_BOX( '+' )
#define	VD_HORIZ	IBM_BOX( 196 )	OTHER_BOX( '-' )
#define	VD_CEN		IBM_BOX( 215 )	OTHER_BOX( '+' )
#define	VD_LR		IBM_BOX( 189 )	OTHER_BOX( '+' )
#define	VD_UL		IBM_BOX( 214 )	OTHER_BOX( '+' )

#endif /* __BOX_H */
