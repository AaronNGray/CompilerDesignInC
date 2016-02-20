/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <ctype.h>
#include <tools/debug.h>

/* ESC.C	Map escape sequences to single characters */

PRIVATE int hex2bin P(( int c ));
PRIVATE int oct2bin P(( int c ));
/*------------------------------------------------------------*/

#define ISHEXDIGIT(x) (isdigit(x)||('a'<=(x)&&(x)<='f')||('A'<=(x)&&(x)<='F'))
#define ISOCTDIGIT(x) ('0'<=(x) && (x)<='7')

PRIVATE int hex2bin(c)
int	c;
{
    /* Convert the hex digit represented by 'c' to an int. 'c' must be one of
     * the following characters: 0123456789abcdefABCDEF
     */
    return (isdigit(c) ? (c)-'0': ((toupper(c))-'A')+10)  & 0xf;
}

PRIVATE int oct2bin(c)
int	c;
{
    /* Convert the hex digit represented by 'c' to an int. 'c' must be a
     * digit in the range '0'-'7'.
     */
    return ( ((c)-'0')  &  0x7 );
}

/*------------------------------------------------------------*/

PUBLIC  int	esc(s)
char	**s;
{
    /* Map escape sequences into their equivalent symbols. Return the equivalent
     * ASCII character. *s is advanced past the escape sequence. If no escape
     * sequence is present, the current character is returned and the string
     * is advanced by one. The following are recognized:
     *
     *	\b	backspace
     *	\f	formfeed
     *	\n	newline
     *	\r	carriage return
     *	\s	space
     *	\t	tab
     *	\e	ASCII ESC character ('\033')
     *	\DDD	number formed of 1-3 octal digits
     *	\xDDD	number formed of 1-3 hex digits
     *	\^C	C = any letter. Control code
     */

    int	rval;

    if( **s != '\\' )
	rval = *( (*s)++ );
    else
    {
	++(*s);					/* Skip the \ */
	switch( toupper(**s) )
	{
	case '\0':  rval = '\\';		break;
	case 'B':   rval = '\b' ;		break;
	case 'F':   rval = '\f' ;		break;
	case 'N':   rval = '\n' ;		break;
	case 'R':   rval = '\r' ;		break;
	case 'S':   rval = ' '  ;		break;
	case 'T':   rval = '\t' ;		break;
	case 'E':   rval = '\033';		break;

	case '^':   rval = *++(*s) ;
		    rval = toupper(rval) - '@' ;
		    break;

	case 'X':   rval = 0;
		    ++(*s);
		    if( ISHEXDIGIT(**s) )
		    {
			rval  = hex2bin( *(*s)++ );
		    }
		    if( ISHEXDIGIT(**s) )
		    {
			rval <<= 4;
			rval  |= hex2bin( *(*s)++ );
		    }
		    if( ISHEXDIGIT(**s) )
		    {
			rval <<= 4;
			rval  |= hex2bin( *(*s)++ );
		    }
		    --(*s);
		    break;

	default:    if( !ISOCTDIGIT(**s) )
			rval = **s;
		    else
		    {
			++(*s);
			rval = oct2bin( *(*s)++ );
			if( ISOCTDIGIT(**s) )
			{
			    rval <<= 3;
			    rval  |= oct2bin( *(*s)++ );
			}
			if( ISOCTDIGIT(**s) )
			{
			    rval <<= 3;
			    rval  |= oct2bin( *(*s)++ );
			}
			--(*s);
		    }
		    break;
	}
	++(*s);
    }
    return rval;
}
