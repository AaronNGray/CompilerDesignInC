/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __CURSES_H
#define __CURSES_H

typedef struct _window_
{
    int  x_org;	     		/* X coordinate of upper-left corner     */
    int  y_org;	     		/* Y coordinate of upper-left corner     */
    int  x_size;     		/* Horizontal size of text area.	 */
    int  y_size;     		/* Vertical   size of text area.	 */
    int  row;	     		/* Current cursor row    (0 to y_size-1) */
    int  col;	     		/* Current cursor column (0 to x_size-1) */
    void *image;     		/* Image buffer. Holds what used to be   */
		     		/* under the window.			 */
    unsigned wrap_ok   :1 ; 	/* Line wrap is enabled in this win.     */
    unsigned scroll_ok :1 ; 	/* Scrolling permitted in this window    */
    unsigned hidden    :1 ; 	/* Window is hidden (nonstandard)        */
    unsigned boxed     :1 ; 	/* Window is boxed  (nonstandard)        */
    unsigned attrib    :8 ;	/* attribute used for character writes   */

} WINDOW;


#define	bool	unsigned int
#define	reg	register
#define	TRUE	(1)
#define	FALSE	(0)
#define	ERR	(0)
#define	OK	(1)

/*------------------------------------------------------------
 * The following macros implement many of the curses functions.
 */

#define getyx( win, y, x )  \
		    ((x) = ((WINDOW*)(win))->col, (y) = ((WINDOW*)(win))->row)

#define refresh()
#define scrollok(win,flag) ((win)->scroll_ok = (flag))
#define wrapok(win,flag)   ((win)->wrap_ok   = (flag))
#define wrefresh(win)	   /* empty */

/*----------------------------------------------------------
 * Nonstandard Macros: movewin() moves the window relative to the current
 * 		       position. Negative is left or up, positive is right or
 * down. ground() changes the fore and background colors for subsequent writes
 * to the window.
 */

#define mvwinr(w,dy,dx) mvwin((w),((w)->y_org - (w)->boxed) + (dy), \
				  ((w)->x_org - (w)->boxed) + (dx) )

#define ground(win,f,b)    ( win->attrib = ((f) & 0x7f) | ((b) & 0x7f) << 4)

/*---------------------------------------------------------
 * Externs for the window functions and #defines to map the standard screen
 * functions to the stdscr versions. There are a few idiosyncrasies here.
 * In particular, mvcur() just ignores it's first two arguments and maps to a
 * move() call. Similarly, subwin() just maps to a newwin() call, and clearok()
 * isn't supported. You must clear the window explicitly before writing to it.
 */

extern  WINDOW 		*stdscr;
extern	void endwin	(void);
extern  void initscr	(void);
extern	int waddch	(WINDOW *, int);

#define	addch(c)        waddch(stdscr, c)

extern	void waddstr	(WINDOW *, char *);
#define addstr(s)	waddstr(stdscr,s)

extern	void wclrtoeol	(WINDOW *	);
#define clrtoeol()	wclrtoeol(stdscr)

extern	void werase	(WINDOW *	);
#define erase		werase(stdscr)

#define wclear(w)	werase(w)
#define clear()		werase(stdscr)

extern	int wgetch	(WINDOW *);
#define getch()		wgetch( stdscr )

extern	void wmove	 (WINDOW *, int, int );
#define move(y,x)	 wmove( stdscr,(y),(x)  )
#define mvcur(oy,ox,y,x) move((y),(x))

extern	int wprintw	(WINDOW *, char *, ...);
extern	int printw	(          char *, ...);

extern  int wscroll	(WINDOW	*, int	       );
#define	scroll(win)	wscroll(win,1)

extern  int winch	(WINDOW *);
#define inch()		winch(stdscr)
#define mvinch(y,x)	( wmove(stdscr,y,x), winch(stdscr) )
#define mvwinch(w,y,x)  ( wmove(w,     y,x), winch(w)      )

extern	WINDOW *newwin	(int ,int ,int ,int );
#define subwin(w,a,b,c,d)  newwin(a,b,c,d)

/*----------------------------------------------------------
 * Externs for functions that don't have stdscr versions
 */

extern void box		( WINDOW *,int ,int );		/* UNIX functions */
extern void crmode	( void);
extern void delwin	( WINDOW * );
extern void echo	( void);
extern  int mvwin	( WINDOW *win, int y, int x );
extern void nl		( void );
extern void nocrmode	( void );
extern void noecho	( void );
extern void nonl	( void );
extern void boxed	( void );		/* Nonstandard functions */
extern void unboxed	( void );
extern void save	( void );
extern void nosave	( void );
extern void def_ground	( int, int );

#endif /* __CURSES_H */
