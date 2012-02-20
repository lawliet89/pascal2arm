#include <stdio.h>

#ifdef __thumb
/* Define Angel Semihosting SWI to be Thumb one */
#define SemiSWI 0xAB
#else
/* Define Angel Semihosting SWI to be ARM one */
#define SemiSWI 0x123456
#endif

/* We use the following Debug Monitor SWIs in this example */

/* Write a character */ 
__swi(SemiSWI) void _WriteC(unsigned op, const char *c);
#define WriteC(c) _WriteC (0x3,c)

/* Exit */
__swi(SemiSWI) void _Exit(unsigned op, unsigned except);
#define Exit() _Exit (0x18,0x20026)

void Write0 (const char *string)
{ int pos = 0;
  while (string[pos] != 0)
    WriteC(&string[pos++]);
}

void C_Entry(void)
{
    int i;
    char buf[20];

    for (i = 0; i < 10; i++) {
          sprintf(buf, "Hello, World %d\n", i);
          Write0(buf);
    }
    Exit();
}
