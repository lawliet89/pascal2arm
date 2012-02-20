/*
 * RCS $Revision: 1.1.2.1 $
 * Checkin $Date: 1998/03/13 16:59:33 $
 * Revising $Author: ijohnson $
 */

/* Random number generator demo program

   Calls assembler function 'randomnumber' defined in random.s
*/

#include <stdio.h>

/* this function prototype is needed because 'randomnumber' is external */
extern unsigned int randomnumber( void );

int main()
{
  int loop;

  for( loop = 0; loop < 10; loop++ )
  { printf( "randomnumber() returned %08x\n", randomnumber() );
  }

  return( 0 );
}
