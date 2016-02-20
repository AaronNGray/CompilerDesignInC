/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <ctype.h>
#include <tools/compiler.h>	/* for stoul and stol prototypes only */

unsigned long	stoul(instr)
char	**instr;
{
    /*	Convert string to long. If string starts with 0x it is interpreted as
     *  a hex number, else if it starts with a 0 it is octal, else it is
     *  decimal. Conversion stops on encountering the first character which is
     *  not a digit in the indicated radix. *instr is updated to point past the
     *  end of the number.
     */

    unsigned long num  = 0 ;
    char	  *str = *instr;

    while( isspace(*str) )
	++*str ;

    if(*str != '0')
    {
	while( isdigit(*str) )
	    num = (num * 10) + (*str++ - '0');
    }
    else
    {
	if (*++str == 'x'  ||  *str == 'X')		/* hex  */
	{
	    for( ++str; isxdigit(*str); ++str )
		num = (num * 16) + ( isdigit(*str) ? *str - '0'
						   : toupper(*str) - 'A' + 10 );
	}
	else
	{
	    while( '0' <= *str  &&  *str <= '7' )	/* octal */
	    {
		num *= 8;
		num += *str++ - '0' ;
	    }
	}
    }
    *instr = str;
    return( num );
}
/*----------------------------------------------------------------------*/
long	stol(instr)
char	**instr;
{
    /* Like stoul(), but recognizes a leading minus sign and returns a signed
     * long.
     */

    while( isspace(**instr) )
	++*instr ;

    if( **instr != '-' )
	return (long)( stoul(instr) );
    else
    {
	++*instr;				/* Skip the minus sign. */
	return -(long)( stoul(instr) );
    }
}
