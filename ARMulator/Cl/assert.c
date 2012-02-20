/* assert.c: support for assert() macro              */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

void __assert(char *expr, char *file, int line)
{   fprintf(stderr, "*** assertion failed: %s, file %s, line %d\n",
            expr, file, line);
    abort();
}

