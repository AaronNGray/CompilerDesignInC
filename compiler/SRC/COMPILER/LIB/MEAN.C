/*@A (C) 1992 Allen I. Holub                                                */
#include <stdio.h>
#include <math.h>
#include <tools/debug.h>
#include <tools/compiler.h>

/* Functions to compute the mean of a bunch of samples. Call with reset true
 * the first time, thereafter call with reset false and data holding the current
 * sample. Returns the running mean and *dev is modified to hold the standard
 * deviation.
 */

double	mean( reset, sample, dev )
int	reset;
double	sample;
double  *dev;
{
    static double m_xhat, m_ki, d_xhat, d_ki ;
    double mean;

    if( reset )
	 return( m_ki = m_xhat = d_ki = d_xhat = 0.0 );

    m_xhat += (sample - m_xhat) / ++m_ki ;
    mean    = m_xhat;
    *dev    = sqrt( d_xhat += (pow( fabs(mean-sample), 2.0) - d_xhat) / ++d_ki);

    return mean;
}

#ifdef MAIN
main()
{
    double x, m, dev;

    mean( 1, 0 );

    for( x = 1.0; x <= 20.0; x++ )
    {
	m = mean( 0, x, &dev);
	printf("mean of 1...%-10g = %-10g, dev = %-10g\n", x, m, dev );
    }
}
#endif
