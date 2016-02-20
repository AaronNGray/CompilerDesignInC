/*@A (C) 1992 Allen I. Holub                                                */

#ifndef __YYSTACK_H
#define __YYSTACK_H
/* yystack.h	Stack-maintenance macros --- yacc and llama version.
 */

#define yystk_cls    /* empty */

#define yystk_dcl(stack,type,size)  typedef type yyt_##stack;		    \
				    yystk_cls yyt_##stack stack[size];      \
				    yystk_cls yyt_##stack (*yyp_##stack)    \
							    = stack + (size)
#define yystk_clear(stack) (  (yyp_##stack)  = (stack + \
						sizeof(stack)/sizeof(*stack)) )

#define yystk_full(stack)  (  (yyp_##stack) <= stack )
#define yystk_empty(stack) (  (yyp_##stack) >= (stack + \
						sizeof(stack)/sizeof(*stack)) )

#define yystk_ele(stack)   ((sizeof(stack)/sizeof(*stack)) - \
							(yyp_##stack-stack))

#define yystk_item(stack,offset) ( *(yyp_##stack + (offset)) )
#define yystk_p(stack)	   	 yyp_##stack

#define yypush_(stack,x) ( *--yyp_##stack = (x)	)
#define yypop_(stack)	 ( *yyp_##stack++	)

#define yypush(stack,x)    ( yystk_full(stack)				\
				? ((yyt_##stack)(long)(yystk_err(1)))  	\
			        : yypush_(stack,x) 			)

#define yypop(stack)       ( yystk_empty(stack)				\
				? ((yyt_##stack)(long)(yystk_err(0)))	\
			        : yypop_(stack)				)

#define yypopn_(stack,amt) ( (yyp_##stack += amt)[-amt]  )
#define yypopn( stack,amt) ( (yystk_ele(stack) < amt)			\
				? ((yyt_##stack)(long)(yystk_err(0)))	\
			        : yypopn_(stack,amt)			)

#define yystk_err(o)	   ((o) ? ferr("Stack overflow\n" ) \
			        : ferr("Stack underflow\n") )
#endif /* __YYSTACK_H */
