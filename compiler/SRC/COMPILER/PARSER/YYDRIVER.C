/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <stdlib.h>

#include <tools/debug.h>
#include <tools/set.h>
#include <tools/hash.h>
#include <tools/compiler.h>
#include <tools/l.h>
#include "parser.h"

/*----------------------------------------------------------------------*/

void file_header	P(( void ));		/* public */
void code_header	P(( void ));
void driver		P(( void ));

/*----------------------------------------------------------------------*/

PRIVATE FILE	*Driver_file = stderr ;

/*----------------------------------------------------------------------
 * Routines in this file are occs specific. There's a different version of all
 * these routines in lldriver.c. They MUST be called in the following order:
 *			file_header()
 *			code_header()
 *			driver()
 *----------------------------------------------------------------------
 */

PUBLIC	void file_header()
{
    /* This header is printed at the top of the output file, before the
     * definitions section is processed. Various #defines that you might want
     * to modify are put here.
     */

    output( "#include \"%s\"\n\n",  TOKEN_FILE	);

    if( Public )
	output( "#define PRIVATE\n" );

    if( Debug )
	output( "#define YYDEBUG\n" );

    if( Make_actions )
	output( "#define YYACTION\n" );

    if( Make_parser )
	output( "#define YYPARSER\n" );

    if( !( Driver_file = driver_1(Output, !No_lines, Template) ))
	error( NONFATAL, "%s not found--output file won't compile\n", Template);
}

/*----------------------------------------------------------------------*/

PUBLIC	void	code_header()
{
    /* This stuff is output after the definitions section is processed, but
     * before any tables or the driver is processed.
     */

    driver_2( Output, !No_lines );
}

/*----------------------------------------------------------------------*/

PUBLIC	void	driver()
{
    /* Print out the actual parser by copying llama.par to the output file.
     */

    if( Make_parser )
	driver_2( Output, !No_lines );

    fclose( Driver_file );
}
