/*@A (C) 1992 Allen I. Holub                                                */
#include <tools/termlib.h>
#include <tools/vbios.h>

void vb_putc( c, attrib )
int c, attrib;
{
    /* Write a character to the screen in TTY mode. Only normal printing
     * characters, BS, BEL, CR and LF are recognized. The cursor is automatic-
     * ally advanced and lines will wrap. The WRITE_TTY BIOS service doesn't
     * handle attributes correctly, so printing characters have to be output
     * twice---once by VB_OUTCHA to set the attribute bit, and atain using
     * WRITE_TTY to move the cursor. WRITE_TTY picks up the existing attribute.
     */

    if( c != '\b' && c != '\007' && c != '\r' && c != '\n' )
	VB_OUTCHA( (c & 0xff) | (attrib << 8) );

    _Vbios( WRITE_TTY, c, attrib & 0xff, 0, 0, "ax" );
}
