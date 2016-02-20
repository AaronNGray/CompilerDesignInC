/* PROTO.H:    Function prototypes for the various files that
*             comprise the compiler.
*/

#include "label.h"
#include "switch.h"
#include "symtab.h"
#include "value.h"

/* ======================= decl.c ======================== */

extern	link *new_class_spec(int first_char_of_lexeme);
extern	void set_class_bit(int first_char_of_lexeme,link *p);
extern	link *new_type_spec(char *lexeme);
extern	void add_spec_to_decl(link *p_spec,symbol *decl_chain);
extern	void add_symbols_to_table(symbol *sym);
extern	void figure_osclass(symbol *sym);
extern	void generate_defs_and_free_args(symbol *sym);
extern	symbol *remove_duplicates(symbol *sym);
extern	void print_bss_dcl(symbol *sym);
extern	void var_dcl(void (*ofunct)(char *,...),int c_code_sclass,symbol *sym,char *terminator);
extern	int illegal_struct_def(structdef *cur_struct,symbol *fields);
extern	int figure_struct_offsets(symbol *p,int is_struct);
extern	int get_alignment(link *p);
extern	void do_enum(symbol *sym,int val);
extern	int conv_sym_to_int_const(symbol *sym,int val);
extern	void fix_types_and_discard_syms(symbol *sym);
extern	int figure_param_offsets(symbol *sym);
extern	void print_offset_comment(symbol *sym,char *label);

/* ======================= gen.c ======================== */

extern	void gen_comment(char *format,...);
extern	void enable_trace(void);
extern	void disable_trace(void);
extern	void gen(char *op,...);

/* ======================= local.c ======================== */

extern	void loc_reset(void);
extern	int loc_var_space(void);
extern	void figure_local_offsets(symbol *sym,char *funct_name);
extern	void loc_auto_create(symbol *sym);
extern	void create_static_locals(symbol *sym,char *funct_name);
extern	void loc_static_create(symbol *sym,char *funct_name);
extern	void remove_symbols_from_table(symbol *sym);

/* ======================= main.c ======================== */

extern	void main(int argc,char **argv );
extern	void yyhook_a(void);
extern	void yyhook_b(void);

/* ======================= op.c ======================== */

extern	value *do_name(char *yytext,symbol *sym);
extern	symbol *make_implicit_declaration(char *name,symbol **undeclp );
extern	void purge_undecl(void);
extern	value *do_unop(int op,value *val);
extern	void do_unary_const(int op,value *val);
extern	int tf_label(void);
extern	value *gen_false_true(int labelnum,value *val);
extern	value *incop(int is_preincrement,int op,value *val);
extern	value *addr_of(value *val);
extern	value *indirect(value *offset,value *ptr);
extern	value *do_struct(value *val,int op,char *field_name);
extern	value *call(value *val,int nargs);
extern	char *ret_reg(link *p);
extern	value *assignment(int op,value *dst,value *src);
extern	void or(value *val,int label);
extern	value *gen_rvalue(value *val);
extern	void and(value *val,int label);
extern	value *relop(value *v1,int op,value *v2);
extern	value *binary_op(value *v1,int op,value *v2);
extern	value *plus_minus(value *v1,int op,value *v2);
extern	int rlabel(int incr);

/* ======================= symtab.c ======================== */

extern	symbol *new_symbol(char *name,int scope);
extern	void discard_symbol(symbol *sym);
extern	void discard_symbol_chain(symbol *sym);
extern	link *new_link(void);
extern	void discard_link_chain(link *p);
extern	void discard_link(link *p);
extern	structdef *new_structdef(char *tag);
extern	void add_declarator(symbol *sym,int type);
extern	void spec_cpy(link *dst,link *src);
extern	link *clone_type(link *tchain,link **endp );
extern	int the_same_type(link *p1,link *p2,int relax);
extern	int get_sizeof(link *p);
extern	symbol *reverse_links(symbol *sym);
extern	char *sclass_str(int class);
extern	char *oclass_str(int class);
extern	char *noun_str(int noun);
extern	char *attr_str(specifier *spec_p);
extern	char *type_str(link *link_p);
extern	char *tconst_str(link *type);
extern	char *sym_chain_str(symbol *chain);
extern	void print_syms(char *filename);

/* ======================= switch.c ======================== */

extern	stab *new_stab(value *val,int label);
extern	void add_case(stab *p,int on_this,int go_here);
extern	void add_default_case(stab *p,int go_here);
extern	void gen_stab_and_free_table(stab *p);

/* ======================= temp.c ======================== */

extern	int tmp_alloc(int size);
extern	void tmp_free(int offset);
extern	void tmp_reset(void);
extern	void tmp_freeall(void);
extern	int tmp_var_space(void);

/* ======================= value.c ======================== */

extern	value *new_value(void);
extern	void discard_value(value *p);
extern	char *shift_name(value *val,int left);
extern	char *rvalue(value *val);
extern	char *rvalue_name(value *val);
extern	value *tmp_create(link *type,int add_pointer);
extern	char *get_prefix(link *type);
extern	value *tmp_gen(link *tmp_type,value *src);
extern	char *convert_type(link *targ_type,value *src);
extern	int get_size(link *type);
extern	char *get_suffix(link *type);
extern	void release_value(value *val);
extern	value *make_icon(char *yytext,int numeric_val);
extern	value *make_int(void);
extern	value *make_scon(void);

/* ======================= c.y ======================== */

extern	int stk_err(int o);
extern	int sym_cmp(symbol *s1,symbol *s2);
extern	int struct_cmp(structdef *s1,structdef *s2);
extern	unsigned int sym_hash(symbol *s1);
extern	unsigned int struct_hash(structdef *s1);
extern	void yy_init_occs(void *val);
extern	char *yypstk(void *val,char *name);

