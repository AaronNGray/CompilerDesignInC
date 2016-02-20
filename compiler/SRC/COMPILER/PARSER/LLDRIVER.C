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
extern  void file_header	P(( void ));			/* public */
extern  void code_header	P(( void ));
extern  void driver		P(( void ));
/*----------------------------------------------------------------------*/
PRIVATE FILE	*Driver_file = stderr ;

/*----------------------------------------------------------------------
 * Routines in this file are llama specific. There's a different version
 * of all these routines in yydriver.c.
 *----------------------------------------------------------------------
 */

PUBLIC	void file_header()
{
    /* This header is printed at the top of the output file, before
     * the definitions section is processed. Various #defines that
     * you might want to modify are put here.
     */

    if( Public )
	output( "#define YYPRIVATE\n" );

    if( Debug )
	output( "#define YYDEBUG\n" );

    output( "\n/*-------------------------------------------*/\n\n");

    if( !( Driver_file = driver_1(Output, !No_lines, Template) ))
	error( NONFATAL, "%s not found--output file won't compile\n", Template);

    output( "\n/*-------------------------------------------*/\n\n");

}
/*----------------------------------------------------------------------*/
PUBLIC	void	code_header()
{
    /* This header is output after the definitions section is processed,
     * but before any tables or the driver is processed.
     */

    output( "\n\n/*--------------------------------------*/\n\n");
    output( "#include \"%s\"\n\n",	TOKEN_FILE 		);
    output( "#define YY_MINTERM      1\n" 			);
    output( "#define YY_MAXTERM      %d\n", Cur_term 		);
    output( "#define YY_MINNONTERM   %d\n", MINNONTERM		);
    output( "#define YY_MAXNONTERM   %d\n", Cur_nonterm		);
    output( "#define YY_START_STATE  %d\n", MINNONTERM		);
    output( "#define YY_MINACT       %d\n", MINACT		);
    output( "\n"						);

    driver_2( Output, !No_lines );
}
/*----------------------------------------------------------------------*/
PUBLIC	void	driver()
{
    /* Print out the actual parser by copying the file llama.par
     * to the output file.
     */

    driver_2( Output, !No_lines );
    fclose( Driver_file );
}
