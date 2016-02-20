/*@A (C) 1992 Allen I. Holub                                                */
#ifndef __SWITCH_H
#define __SWITCH_H
#include "value.h"		/* For VALNAME_MAX definition.		*/

#define CASE_MAX 256		/* Maximum number of cases in a switch  */

typedef struct case_val		/* a single dispatch-table element	*/
{
    int	 on_this;		/* The N in a "case N:" statement 	*/
    int  go_here;		/* Numeric component of label in output */
} case_val;			/*				  code. */


typedef struct stab		/* a switch table */
{
    case_val *cur;	  	   /* pointer to next available slot in table */
    case_val table[ CASE_MAX 	]; /* switch table itself.		      */
    char     name [ VALNAME_MAX ]; /* switch on this rvalue		      */
    int	     def_label;		   /* label associated with default case      */
    int	     stab_label;	   /* label at top and bottom of selector     */
				   /* code. Bottom label is stab_label+1.     */
} stab;


#endif /* __SWITCH_H */
