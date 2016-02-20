/*@A (C) 1992 Allen I. Holub                                                */
%union {
   int   op_type;
   char  *var_name;
}

%term  DIVOP					/*  / or %  */
%type <var_name> e statement
%type <op_type>  divop

%%
goal      : statement
          ;

statement : e divop e
 	    {
		if( $2 == '/' )
		    gen( "%s /= %s;", $1, $3 )
		else
		    gen( "%s %= %s;", $1, $3 )

		free_name( $3 );
		$$ = $1
 	    }
 	  ;
divop     : DIVOP  { $$ = *yytext; }
          ;
%%

