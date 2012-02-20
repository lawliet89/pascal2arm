#include <stdio.h>

#include "mul.h"

int main(void)
{
    Unsigned64 u;
    Int64 s;

    u = umull(0xA0000000, 0x10101010);
    printf("umull (0xA0000000*0x10101010) = 0x%08x%08x\n", u.hi, u.lo);
    u = umlal(u, 0x00500000, 0x10101010);
    printf("umlal (+0x00500000*0x10101010) = 0x%08x%08x\n", u.hi, u.lo);
    s = smull(0xA0000000, 0x10101010);
    printf("smull (0xA0000000*0x10101010) = 0x%08x%08x\n", s.hi, s.lo);
    s = smlal(s, 0x00500000, 0x10101010);
    printf("smlal (+0x00500000*0x10101010) = 0x%08x%08x\n", s.hi, s.lo);
}
