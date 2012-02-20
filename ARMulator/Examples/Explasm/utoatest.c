/*
 * RCS $Revision: 1.1.2.1 $
 * Checkin $Date: 1998/03/13 16:59:35 $
 * Revising $Author: ijohnson $
 */

/* Simple test harness for testing utoa (not dtoa) */

#include <stdio.h>

extern char *utoa( char *string, unsigned int num );

int main()
{
  unsigned int num;
  char buffer[ 12 ];

  puts( "Enter number:" );

  if( scanf( "%d", &num ) == 1 )
  {
    *utoa( buffer, num ) = 0; /* add string terminator */
    puts( "utoa yields:" );
    puts( buffer );
  }

  return( 0 );
}
