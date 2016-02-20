#ifndef __COMPILER_H
#define __COMPILER_H
#include <tools/debug.h>
#if (0  ANSI(+1))
#    ifndef va_arg
#        include <stdarg.h>
#    endif
#endif
#include <tools/prnt.h>
#include <tools/set.h>
#include <tools/hash.h>
/* ==================== \src\tools\assort.c ==================== */

extern  void assort P((void * *base,int nel,int elsize,int (*cmp)(void *,void *)));

/* ==================== \src\tools\bintoasc.c ==================== */

extern  char *bin_to_ascii P((int c,int use_hex));

/* ==================== \src\tools\copyfile.c ==================== */

extern  int copyfile P((char *dst,char *src,char *mode));

/* ==================== defnext.c ==================== */

extern  void defnext P((FILE *fp,char *name));

/* ==================== driver.c ==================== */

extern  FILE *driver_1 P((FILE *output,int lines,char *file_name));
extern  int driver_2 P((FILE *output,int lines));

/* ==================== \src\tools\esc.c ==================== */

extern  int hex2bin P((int c));
extern  int oct2bin P((int c));
extern  int esc P((char * *s));

/* ==================== \src\tools\fputstr.c ==================== */

extern  void fputstr P((char *str,int maxlen,FILE *stream));

/* ==================== \src\tools\hashadd.c ==================== */

extern  unsigned int hash_add P((unsigned char *name));

/* ==================== \src\tools\hashpjw.c ==================== */

extern  unsigned int hash_pjw P((unsigned char *name));

/* ==================== \src\tools\mean.c ==================== */

extern  double mean P((int reset,double sample,double *dev));

/* ==================== \src\tools\memiset.c ==================== */

extern  int *memiset P((int *dst,int with_what,int count));

/* ==================== \src\tools\movefile.c ==================== */

extern  int movefile P((char *dst,char *src,char *mode));

/* ==================== pairs.c ==================== */

extern  int pairs P((FILE *fp,int *array,int nrows,int ncols,char *name,int threshold,int numbers));
extern  void pnext P((FILE *fp,char *name));

/* ==================== \src\tools\pchar.c ==================== */

extern  void pchar P((int c,FILE *stream));

/* ==================== print_ar.c ==================== */

extern  void print_array P((FILE *fp,int *array,int nrows,int ncols));

/* ==================== \src\tools\printv.c ==================== */

extern  void printv P((FILE *fp,char * *argv));
extern  void comment P((FILE *fp,char * *argv));

/* ==================== \src\tools\searchen.c ==================== */

extern  void unixfy P((char *str));
extern  int searchenv P((char *filename,char *envname,char *pathname));

/* ==================== \src\tools\set.c ==================== */

extern  SET *newset P((void ));
extern  void delset P((SET *set));
extern  SET *dupset P((SET *set));
extern  int _addset P((SET *set,int bit));
extern  void enlarge P((int need,SET *set));
extern  int num_ele P((SET *set));
extern  int _set_test P((SET *set1,SET *set2));
extern  int setcmp P((SET *set1,SET *set2));
extern  unsigned int sethash P((SET *set1));
extern  int subset P((SET *set,SET *possible_subset));
extern  void _set_op P((int op,SET *dest,SET *src));
extern  void invert P((SET *set));
extern  void truncate P((SET *set));
extern  int next_member P((SET *set));
extern  void pset P((SET *set,int (*output_routine)(void *,char *,int ),void *param));

/* ==================== \src\tools\ssort.c ==================== */

extern  void ssort P((char *base,int nel,int elsize,int (*cmp)(void *,void *)));

/* ==================== \src\tools\stol.c ==================== */

extern  unsigned long stoul P((char * *instr));
extern  long stol P((char * *instr));

/* ==================== \src\tools\concat.c ==================== */

extern  int concat P((int size,char *dst,...));

/* ==================== \src\tools\ferr.c ==================== */

extern  int ferr P((char *fmt,...));

/* ==================== \src\tools\onferr.c ==================== */

extern  int on_ferr P((void ));

#endif /*__COMPILER_H */
