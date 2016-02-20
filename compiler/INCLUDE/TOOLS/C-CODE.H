/*@A (C) 1992 Allen I. Holub                                                */
#define BYTE_WIDTH	1	/* Widths of the basic types.	      */
#define WORD_WIDTH	2
#define LWORD_WIDTH	4
#define PTR_WIDTH	4

#define BYTE_HIGH_BIT  "0xff80"		/* High-bit mask. */
#define WORD_HIGH_BIT  "0x8000"
#define LWORD_HIGH_BIT "0x80000000L"

#define SWIDTH		LWORD_WIDTH	/* Stack width (in bytes).	      */
#define SDEPTH		1024		/* Number of elements in stack.	      */
#define ALIGN_WORST	LWORD_WIDTH	/* Long word is worst-case alignment. */
#define BYTE_PREFIX	"B"	/* Indirect-mode prefixes. */
#define WORD_PREFIX	"W"
#define LWORD_PREFIX	"L"
#define PTR_PREFIX	"P"
#define BYTEPTR_PREFIX	"BP"
#define WORDPTR_PREFIX	"WP"
#define LWORDPTR_PREFIX	"LP"
#define PTRPTR_PREFIX	"PP"
