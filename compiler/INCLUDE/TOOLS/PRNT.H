/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __PRNT_H
#define __PRNT_H

#ifndef va_arg
#include <stdarg.h>
#endif
#include <tools/debug.h>

typedef int (*prnt_t) P(( int, ... ));
void prnt P(( prnt_t ofunct, void *funct_arg, char *format, va_list args ));

void stop_prnt( void );
#endif /* __PRNT_H */
