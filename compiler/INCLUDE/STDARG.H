/*@A (C) 1992 Allen I. Holub                                                */

#ifndef _STDARG_H_		/* suppress multiple #includes */
#define _STDARG_H_

typedef char *va_list;

#define va_start(arg_ptr,first)	arg_ptr = (va_list)&first + sizeof(first)
#define va_arg(arg_ptr,type) 	((type *)(arg_ptr += sizeof(type)))[-1]
#define va_end(arg_ptr)		/* empty */

#endif
