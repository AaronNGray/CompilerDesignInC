	The following changes were made by Herb Norton to the files on the
1.01 distribution disk of software from "Compiler Design in C" to get it to
compile under System V UNIX, comments in double brackets [[like this]] are my
own:

-Allen Holub

----------------------------------------------------------------------
bin directory

1.  In the pp command, the C Shell constructs were replaced with the use
    of basename.

include directory

2.  The curses.h file was renamed to o.curses.h so that the system
    curses.h would be picked up.  Otherwise, an unresolved reference to
    the winch function occurs.

3.  In the search.h file, a semicolon was added to the end of the line
    defining the bsearch function.  The lack of this semicolon caused
    obscure error messages and it was difficult to find.

    [[This is a bug and will be fixed in release 1.02 of the sofware. -ah]]

include/tools directory

4.  In compiler.h all instances of _iobuf were changed to FILE to be
    consistent with declarations elsewhere.
    
    [[ This is probably a bug too, but I've left it alone on the distribution
       disk for 2 reasons: (1) Compiler.h is generated automatically by
       Microsoft C, which doesn't process typedefs correctly when it creates
       prototypes and (2) If you say "struct _iobuf" you don't have to #include
       <stdio.h> before <tools/compiler.h>. This is really not a great idea
       --you should include <stdio.h> first and use FILE, but I don't want to
       break existing code that doesn't do it right. -ah]]

5.  In debug.h the #define of memmove was changed from bcopy to memcpy
    and the #define of memcpy was commented out.

    [[ Note that this change assumes that your memcpy() handles overlapping
       strings correctly. If you have a memmove() that supports overlapping
       strings, then remove the #define of memmove() to memcpy() from
       debug.h.  -ah]]

6.  In hash.h some prototype definitions were removed because of
    duplications.


src/compiler/lib directory

7.  In concat.c the #pragma loop_opt and the surrounding #ifdef and 
    #endif lines were removed.  Even those these lines are only for
    MSDOS, the Unix cpp complains about the #pragma line.

8.  In driver.c the #include of tools/compiler.h was commented out
    because of problems, although this could possible have been related
    to the missing semicolon problem in 3.

9.  In hash.c, the code using ftime was removed, because our system does
    not have an ftime.

10. In yydebug.h, SIGIO was defined as SIGUSR1, <fcntl.h> was included
    to define F_SETL, and the use of FASYNC was eliminated, since it is
    not provided on our system.  The to_log function was moved so it was
    defined before it was used.  The use of ftime was changed to a use
    of time.


src/compiler/lex directory

11. In lex.c, the declaration of the driver_1 function as returning a
    pointer to a FILE was removed, since it was incorrect.
    
    [[ But driver_1() does indeed return a FILE pointer---I'm not sure what
       was going on here.  -ah]]

12. In squash.c, <memory.h> was included.


src/compiler/parser directory

13. In main.c, the code using ftime was removed.  Actually, some was
    changed to time, but the printing was removed.

14. In makefile (the Unix one), follow.o was moved from LLOBJ to COMOBJ
    to resolve references.  When building occs after build the second
    version of llama, the instructions should tell you to touch acts.c
    or the makefile should be changed to force acts.pp.c to be rebuilt
    because there are LL and OX macros that expand differently when you
    are building llama or occs.


src/compiler/c directory

15. In makefile, lexyy.o was moved to the end of OBJ to resolve
    references.  In the call to occs for c.y, the T parameter was added
    to force creation of yyoutab.c which is expected by the makefile.

    [[ The -T was deliberately omitted because I had intended to make the tables
       only once (with the occs -T option), not every time I changed an action.
       Theres a "tables:" dependancy in the makefile for this purpose.
       Since occs is not the world's fastest program, I saw no point in wasting
       time making tables if the grammar had not been changed (ie. if only the
       action code had been changed). Adding the -T will force occs to make the
       tables all the time, thereby slowing down the compilation process if all
       you change is an action in c.y. -ah]]

16. In yyact.c the #define ALLOC was removed because it is already in
    yyout.c and two of them cause multiple definitions.


src/compiler/c and src/compiler/parser directories

17. The .par files say that the path to them should be put in the LIB
    environment, but I was not able to figure out how to do this, so I
    ended up copying them the the directories where they were needed.

    [[ You do this in the C shell with:

	    setenv LIB <path_name>

       In the Bourne and Korn shells, use the following:

	   LIB=<path_name>
	   export LIB

   -ah ]]

******************************************************************************

Diff of c makefile

12,13c12,13
< OBJ = 	decl.o gen.o local.o main.o op.o switch.o \
< 	symtab.o temp.o value.o yyact.o yyout.o yyoutab.o lexyy.o
---
> OBJ = 	decl.o gen.o lexyy.o local.o main.o op.o switch.o \
> 	symtab.o temp.o value.o yyact.o yyout.o yyoutab.o
32,33c32,33
< yyout.c:	c.y
< 		$(OCCS) -vlWDSTp c.y
---
> _yyout.c:	c.y
> 		$(OCCS) -vlWDSp c.y

******************************************************************************

Diff of compiler.h

15c15
< extern  void defnext P((struct FILE *fp,char *name));
---
> extern  void defnext P((struct _iobuf *fp,char *name));
19,20c19,20
< extern  struct FILE *driver_1 P((struct FILE *output,int lines,char *file_name));
< extern  int driver_2 P((struct FILE *output,int lines));
---
> extern  struct _iobuf *driver_1 P((struct _iobuf *output,int lines,char *file_name));
> extern  int driver_2 P((struct _iobuf *output,int lines));
30c30
< extern  void fputstr P((char *str,int maxlen,struct FILE *stream));
---
> extern  void fputstr P((char *str,int maxlen,struct _iobuf *stream));
66,67c66,67
< extern  int pairs P((struct FILE *fp,int *array,int nrows,int ncols,char *name,int threshold,int numbers));
< extern  void pnext P((struct FILE *fp,char *name));
---
> extern  int pairs P((struct _iobuf *fp,int *array,int nrows,int ncols,char *name,int threshold,int numbers));
> extern  void pnext P((struct _iobuf *fp,char *name));
71c71
< extern  void pchar P((int c,struct FILE *stream));
---
> extern  void pchar P((int c,struct _iobuf *stream));
75c75
< extern  void print_array P((struct FILE *fp,int *array,int nrows,int ncols));
---
> extern  void print_array P((struct _iobuf *fp,int *array,int nrows,int ncols));
79,80c79,80
< extern  void printv P((struct FILE *fp,char * *argv));
< extern  void comment P((struct FILE *fp,char * *argv));
---
> extern  void printv P((struct _iobuf *fp,char * *argv));
> extern  void comment P((struct _iobuf *fp,char * *argv));

******************************************************************************

Diff of concat.c

5a6,9
> #ifdef MSDOS
> #pragma loop_opt(off)  /* Can't do loop optimizations (alias problems) */
> #endif
> 

******************************************************************************

Diff of debug.h

28,29c28,29
< #	define memmove(d,s,n) memcpy(s,d,n)
< /* #	define memcpy(d,s,n)  bcopy(s,d,n) */
---
> #	define memmove(d,s,n) bcopy(s,d,n)
> #	define memcpy(d,s,n)  bcopy(s,d,n)

******************************************************************************

 Diff of driver.c

7c7
< /* #include <tools/compiler.h>	/* for prototypes */
---
> #include <tools/compiler.h>	/* for prototypes */

******************************************************************************

Diff of hash.c

461a462,463
>     ftime( &start_time );
> 
485a488,491
>     ftime( &end_time );
>     time1 = (start_time.time * 1000) + start_time.millitm ;
>     time2 = (  end_time.time * 1000) +   end_time.millitm ;
>     printf( "Elapsed time = %g seconds\n", (time2-time1) / 1000 );

******************************************************************************

Diff of hash.h

22a23
> extern HASH_TAB *maketab P(( unsigned maxsym, unsigned (*hash)(), int(*cmp)()));
28a30,32
> extern int      ptab     P(( HASH_TAB *tabp, void(*prnt)(), void *par, int srt));
> unsigned hash_add	P(( unsigned char *name ));	/* in hashadd.c */
> unsigned hash_pjw	P(( unsigned char *name ));	/* in hashpjw.c */

******************************************************************************

Diff of lex.c

190c190
<     FILE   *input; 		/* Template file for driver	*/
---
>     FILE   *input, *driver_1();	/* Template file for driver	*/

******************************************************************************

Diff of lldriver.c

79d78
<     printf("Finished driver_2");

******************************************************************************

Diff of main.c

298,299c298,299
<     long *start_time, *end_time ;
<     long dtime;
---
>     struct timeb start_time, end_time ;
>     long	 time;
301c301
<     time( &start_time );    /* Initialize times now so that the difference   */
---
>     ftime( &start_time );    /* Initialize times now so that the difference   */
326c326
< 	time( &start_time );
---
> 	ftime( &start_time );
334c334
< 	time  ( &end_time        );
---
> 	ftime  ( &end_time        );
340a341,348
>     }
> 
>     if( Verbose )
>     {
> 	time  = (  end_time.time * 1000) +   end_time.millitm ;
> 	time -= (start_time.time * 1000) + start_time.millitm ;
> 	printf( "time required to make tables: %ld.%-03ld seconds\n",
> 						(time/1000), (time%1000));

******************************************************************************

Diff of parser makefile

53,54c53,54
< COMOBJ = main.o acts.o lexyy.o first.o stok.o follow.o
< LLOBJ  = llselect.o llcode.o lldriver.o lldollar.o
---
> COMOBJ = main.o acts.o lexyy.o first.o stok.o
> LLOBJ  = llselect.o llcode.o lldriver.o follow.o lldollar.o

******************************************************************************

Diff of pp

14c14,15
< echo making $1.pp.c
---
> set fname=$1:t
> echo making $fname:r.pp.c
16c17
< 	    | sed 's/^# \([0-9]\)/#line \1/' > `basename $1 '.c'`.pp.c
---
> 	    | sed 's/^# \([0-9]\)/#line \1/' > $fname:r.pp.c

******************************************************************************

Diff of search.h

14c14
< extern char *bsearch( );
---
> extern char *bsearch( )

******************************************************************************

Diff of squash.c

8d7
< #    include <memory.h>

******************************************************************************

Diff of yyact.c

21a22
> #define ALLOC		/* Define ALLOC to create symbol table in symtab.h.  */

******************************************************************************

diff of yydebug.c

2,3d1
< #define SIGIO SIGUSR1
< 
6d3
< #include <fcntl.h>
208c205
<     UX( fcntl( fileno(stdin), F_SETFL, flags ); )
---
>     UX( fcntl( fileno(stdin), F_SETFL, flags | FASYNC ); )
920,972d916
< 
< /*----------------------------------------------------------------------*/
< 
< PRIVATE  FILE	*to_log( buf )
< char	*buf;
< {
<     /* Set up everything to log output to a file (open the log file, etc.). */
< 
<     if( !yyprompt("Log-file name (CR for \"log\", ESC cancels): ", buf,1) )
< 	return NULL;
< 
<     if( !*buf )
< 	strcpy( buf, "log" );
< 
<     if( !(Log = fopen( buf, "w")) )
<     {
<         NEWLINE(Prompt_window );
< 	wprintw(Prompt_window, "Can't open %s", buf );
< 	presskey();
< 	return NULL;
<     }
< 
<     if( !yyprompt("Log comment-window output? (y/n, CR=y): ", buf,0) )
< 	return NULL;
<     else
< 	No_comment_pix = (*buf == 'n');
< 
<     if( !yyprompt( "Print stack pictures in log file? (y/n, CR=y): ",buf,0) )
< 	return NULL;
< 
<     if( !(No_stack_pix = (*buf == 'n')) )
<     {
< 	if( !yyprompt( "Print stacks horizontally? (y/n, CR=y): ",buf,0) )
< 	    return NULL;
< 
< 	if( Horiz_stack_pix = (*buf != 'n') )
< 	{
< 	    if( !yyprompt("Print SYMBOL stack (y/n, CR=y): ",buf,0) )
< 		return NULL;
< 	    Sym_pix = (*buf != 'n');
< 
< 	    if( !yyprompt("Print PARSE  stack (y/n, CR=y): ",buf,0) )
< 		return NULL;
< 	    Parse_pix = (*buf != 'n');
< 
< 	    if( !yyprompt("Print VALUE  stack (y/n, CR=y): ",buf,0) )
< 		return NULL;
< 	    Attr_pix = (*buf != 'n');
< 	}
<     }
<     return Log;
< }
< 
987c931
<     long time_buf;
---
>     struct timeb time_buf;		/* defined in sys/timeb.h */
1010,1011c954,955
< 	time( &time_buf );
< 	start   = time_buf;
---
> 	ftime( &time_buf );
> 	start   = (time_buf.time * 1000) + time_buf.millitm;
1015,1016c959,960
< 	    time( &time_buf );
< 	    current = time_buf;
---
> 	    ftime( &time_buf );
> 	    current = (time_buf.time * 1000) + time_buf.millitm;
1379a1324,1376
> 
> /*----------------------------------------------------------------------*/
> 
> PRIVATE  FILE	*to_log( buf )
> char	*buf;
> {
>     /* Set up everything to log output to a file (open the log file, etc.). */
> 
>     if( !yyprompt("Log-file name (CR for \"log\", ESC cancels): ", buf,1) )
> 	return NULL;
> 
>     if( !*buf )
> 	strcpy( buf, "log" );
> 
>     if( !(Log = fopen( buf, "w")) )
>     {
>         NEWLINE(Prompt_window );
> 	wprintw(Prompt_window, "Can't open %s", buf );
> 	presskey();
> 	return NULL;
>     }
> 
>     if( !yyprompt("Log comment-window output? (y/n, CR=y): ", buf,0) )
> 	return NULL;
>     else
> 	No_comment_pix = (*buf == 'n');
> 
>     if( !yyprompt( "Print stack pictures in log file? (y/n, CR=y): ",buf,0) )
> 	return NULL;
> 
>     if( !(No_stack_pix = (*buf == 'n')) )
>     {
> 	if( !yyprompt( "Print stacks horizontally? (y/n, CR=y): ",buf,0) )
> 	    return NULL;
> 
> 	if( Horiz_stack_pix = (*buf != 'n') )
> 	{
> 	    if( !yyprompt("Print SYMBOL stack (y/n, CR=y): ",buf,0) )
> 		return NULL;
> 	    Sym_pix = (*buf != 'n');
> 
> 	    if( !yyprompt("Print PARSE  stack (y/n, CR=y): ",buf,0) )
> 		return NULL;
> 	    Parse_pix = (*buf != 'n');
> 
> 	    if( !yyprompt("Print VALUE  stack (y/n, CR=y): ",buf,0) )
> 		return NULL;
> 	    Attr_pix = (*buf != 'n');
> 	}
>     }
>     return Log;
> }
