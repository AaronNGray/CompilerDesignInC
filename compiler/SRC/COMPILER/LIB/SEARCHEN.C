/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <io.h>
#include <string.h>
#include <tools/debug.h>
#include <tools/compiler.h>

#if( 0 BCC(+1) )		/* Borland C/C++	*/
#include <dir.h>		/* getcwd() prototype   */
#endif

#if( 0 MSC(+1) )		/* Microsoft C		*/
#include <direct.h>		/* getcwd() prototype   */
#endif

#define PBUF_SIZE 129  		/* Maximum length of a path name + 1 */

#if (0 UNIX(+1))
    define unixfy(x)	/* empty */
#else
    static void unixfy( char *str )		/* convert name to lower case */
    {						/* and convert \'s to /'s     */
	for(; *str; ++str )
	    if( (*str = tolower(*str)) == '\\' )
		*str = '/';
    }
#endif
/*----------------------------------------------------------------------*/
int	searchenv( filename, envname, pathname )
char	*filename;	/* file name to search for			*/
char	*envname;	/* environment name to use as PATH		*/
char	*pathname;	/* Place to put full path name when found	*/
{
    /* Search for file by looking in the directories listed in the envname
     * environment. Put the full path name (if you find it) into pathname.
     * Otherwise set *pathname to 0. Unlike the DOS PATH command (and the
     * microsoft _searchenv), you can use either a space or semicolon
     * to separate directory names. The pathname array must be at least
     * 128 characters. If the file is in the current directory, the path
     * name of the current directory is appended to the front of the name.
     *
     * Unlike the Microsoft version, this one returns 1 on success, 0 on failure
     */

    char  pbuf[PBUF_SIZE];
    char  *p ;
    MS( int disk; )

    getcwd( pathname, PBUF_SIZE );
    MS( disk = tolower( *pathname ); )

    concat( PBUF_SIZE, pathname, pathname, "/", filename, NULL );
    if( access( pathname, 0 ) != -1 ) 		/* check current directory */
    {
	unixfy( pathname );
	return 1;				/* ...it's there.	   */
    }

    /* The file doesn't exist in the current directory. If a specific path was
     * requested (ie. file contains \ or /) or if the environment isn't set,
     * return failure, otherwise search for the file on the path.
     */

    if( strpbrk(filename,"\\/")  ||  !(p = getenv(envname)) )
	return( *pathname = '\0');

    strncpy( pbuf, p, PBUF_SIZE );
    if( p = strtok( pbuf, "; " ) )
    {
	do
	{
	    MS( if( !p[1] || p[1] != ':' )		       		   )
            MS(	    sprintf( pathname,"%c:%0.90s/%0.20s",disk,p,filename); )
	    MS( else							   )
	            sprintf( pathname, "%0.90s/%0.20s", p, filename );

	    if( access( pathname, 0 ) >= 0 )
	    {
		unixfy( pathname );
		return 1;				/* found it */
	    }
	}
	while( p = strtok( NULL, "; ") );
    }
    return( *pathname = '\0' );
}
   /*----------------------------------------------------------------------*/
#ifdef MAIN
   main( int argc, char **argv )
   {
       char target_buf[256];
       if( searchenv(argv[1], "PATH", target_buf) )
   	   printf("found %s at %s\n", argv[1], target_buf );
       else
   	   printf("Couldn't find %s\n", argv[1] );
   }
#endif
