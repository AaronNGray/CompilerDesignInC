#ifndef __L_H
#define __L_H
#include <tools/debug.h>
#if (0  ANSI(+1))
#    ifndef va_arg
#        include <stdarg.h>
#    endif
#endif
#include <tools/prnt.h>
/* ==================== input.c ==================== */

extern  void ii_io P((int (*open_funct)(char *,int ),int (*close_funct)(int ),int (*read_funct)(int ,void *,unsigned int )));
extern  int ii_newfile P((char *name));
extern  unsigned char *ii_text P((void ));
extern  int ii_length P((void ));
extern  int ii_lineno P((void ));
extern  unsigned char *ii_ptext P((void ));
extern  int ii_plength P((void ));
extern  int ii_plineno P((void ));
extern  unsigned char *ii_mark_start P((void ));
extern  unsigned char *ii_mark_end P((void ));
extern  unsigned char *ii_move_start P((void ));
extern  unsigned char *ii_to_mark P((void ));
extern  unsigned char *ii_mark_prev P((void ));
extern  int ii_advance P((void ));
extern  int ii_flush P((int force));
extern  int ii_fillbuf P((unsigned char *starting_at));
extern  int ii_look P((int n));
extern  int ii_pushback P((int n));
extern  void ii_term P((void ));
extern  void ii_unterm P((void ));
extern  int ii_input P((void ));
extern  void ii_unput P((int c));
extern  int ii_lookahead P((int n));
extern  int ii_flushbuf P((void ));

/* ==================== yypstk.c ==================== */

extern  char *yypstk P((void *val,char *sym));

/* ==================== yywrap.c ==================== */

extern  int yywrap P((void ));

/* ==================== yyhook_a.c ==================== */

extern  void yyhook_a P((void ));

/* ==================== yyhook_b.c ==================== */

extern  void yyhook_b P((void ));

/* ==================== yyinitlx.c ==================== */

extern  void yy_init_lex P((void ));

/* ==================== yyinitox.c ==================== */

extern  void yy_init_occs P((void *tos));

/* ==================== yyinitll.c ==================== */

extern  void yy_init_llama P((void *tos));

/* ==================== yydebug.c ==================== */

extern  int conv P((int c));
extern  int yy_init_debug P((int *sstack,int * *p_sp,char * *dstack,char * * *p_dsp,void *vstack,int v_ele_size,int depth));
extern  void die_a_horrible_death P((void ));
extern  void yy_quit_debug P((void ));
extern  int yy_get_args P((int argc,char * *argv));
extern  void yy_output P((int where,char *fmt,va_list args));
extern  void yycomment P((char *fmt,...));
extern  void yyerror P((char *fmt,...));
extern  void yy_input P((char *fmt,...));
extern  void display_file P((char *name,int buf_size,int print_lines));
extern  void write_screen P((char *filename));
extern  void yy_pstack P((int do_refresh,int print_it));
extern  void yy_redraw_stack P((void ));
extern  void delay P((void ));
extern  void cmd_list P((void ));
extern  int yy_nextoken P((void ));
extern  void yy_break P((int production_number));
extern  int breakpoint P((void ));
extern  int new_input_file P((char *buf));
extern  FILE *to_log P((char *buf));
extern  int input_char P((void ));
extern  int yyprompt P((char *prompt,char *buf,int getstring));
extern  void presskey P((void ));

/* ==================== \src\tools\concat.c ==================== */

extern  int concat P((int size,char *dst,...));

/* ==================== \src\tools\ferr.c ==================== */

extern  int ferr P((char *fmt,...));

/* ==================== \src\tools\onferr.c ==================== */

extern  int on_ferr P((void ));

#endif /* __L_H */
