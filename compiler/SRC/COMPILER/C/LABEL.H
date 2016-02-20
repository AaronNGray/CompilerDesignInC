/*@A (C) 1992 Allen I. Holub                                                */

/* This file contains definitions for the various label prefixes. All labels
 * take the form: <prefix><number>, the <number> supplied by the code-generation
 * action. The prefixes are defined here.
 */
#ifndef __LABEL_H
#define __LABEL_H

#define L_BODY		"BDY"	/* Top of the body of a for loop.	      */
#define L_COND_END	"QE"	/* End of conditional.			      */
#define L_COND_FALSE	"QF"	/* True part of conditional (?:).	      */
#define L_DOEXIT	"DXIT"	/* Just after the end of the do/while.	      */
#define L_DOTEST	"DTST"	/* Just above the test in a do/while.	      */
#define L_DOTOP		"DTOP"	/* Top of do/while loop.		      */
#define L_ELSE		"EL"	/* Used by else processing.		      */
#define L_END		"E"	/* End of relational/logical op.	      */
#define L_FALSE		"F"	/* False target of relational/logical op.     */
#define L_INCREMENT	"INC"	/* Just above the increment part of for loop. */
#define L_LINK		"L"	/* Offset passed to link instruction.	      */
#define L_NEXT		"EXIT"	/* Outside of loop, end of if clause.	      */
#define L_RET		"RET"   /* Above clean-up code at end of subroutine.  */
#define L_STRING	"S"	/* Strings.				      */
#define L_SWITCH	"SW"	/* Used for switches.			      */
#define L_TEST		"TST"	/* Above test in while/for/if.		      */
#define L_TRUE		"T"	/* True target of relational/logical operator.*/
#define L_VAR		"V"	/* Local-static variables.		      */

#endif /* __LABEL_H */

