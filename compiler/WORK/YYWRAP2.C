/*@A (C) 1992 Allen I. Holub                                                */

   /* This is the alternate form of yywrap from Appendix E. It handles multiple
    * input files.
    */

int  Argc;				/* Copy of argc as passed to main(). */
char **Argv;				/* Copy of argv as passed to main(). */

yywrap()
{
    if( --Argc >= 0 )
    {
	if( ii_newfile( *Argv ) != -1 )
	{
	    ++Argv;
	    return 0;			/* New file opened successfully. */
	}
	fprintf(stderr, "Can't open %s\n", *Argv );
    }
    return 1;
}

main( argc, argv )
int	argc;
char	**argv;
{
    Argc = argc - 1;
    Argv = argv + 1;
    ii_newfile( *argv )
    while( yylex() )
	;				/* Discard all input tokens. */
}
