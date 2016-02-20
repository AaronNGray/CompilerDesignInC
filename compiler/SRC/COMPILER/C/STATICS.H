/* ======================= decl.c ======================== */
/* ======================= gen.c ======================== */
static	int cmp(struct ltab *a,struct ltab *b);
static	void print_instruction(char *b,int t);
/* ======================= local.c ======================== */
/* ======================= main.c ======================== */
/* ======================= op.c ======================== */
static	struct symbol *find_field(struct structdef *s,char *field_name);
static	char *access_with(struct value *val);
static	int make_types_match(struct value **v1p ,struct value **v2p );
static	int do_binary_const(struct value **v1p ,int op,struct value **v2p );
static	void dst_opt(struct value **leftp ,struct value **rightp ,int commutative);
/* ======================= symtab.c ======================== */
static	void psym(struct symbol *sym_p,struct _iobuf *fp);
static	void pstruct(struct structdef *sdef_p,struct _iobuf *fp);
/* ======================= switch.c ======================== */
/* ======================= temp.c ======================== */
/* ======================= value.c ======================== */
/* ======================= c.y ======================== */
static	void init_output_streams(char **p_code ,char **p_data ,char **p_bss );
static	void sigint_handler(void);
static	void clean_up(void);
static	int tcmp(struct tabtype *p1,struct tabtype *p2);
