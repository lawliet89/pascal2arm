/* backtrace.c: call stack backtrace                  */
/* Copyright (C) Advanced Risc Machines Ltd., 1991    */

#include <stdio.h>
#include <stdlib.h>
#include "interns.h"
#include "externs.h"

static void *top_of_stack;          /* needs rework for stack extension */

static void *_codebase, *_codelimit;

void _backtrace_init(void *topofstack, void *codebase, void *codelimit) {
    top_of_stack = topofstack;
    _codebase = codebase;
    _codelimit = codelimit;
}

static void p_in(__rt_unwindblock *uwb)
{    fprintf(stderr, "%x in ", uwb->pc & ~0xfc000003);
}

#define ERROR_ILLEGALREAD 0x80800ea0
#define ERROR_ILLEGALWRITE 0x80800ea1

void _backtrace(int why, int *address, __rt_unwindblock *uwb)
{   /* all the messages in the following should go to stderr             */
    FILE *err = stderr;
    if (why==ERROR_ILLEGALREAD || why == ERROR_ILLEGALWRITE)
       fprintf(err, "(address %p = %d)\n\n",
               address, (int)address);
    else
        fprintf(err, "\nPostmortem requested\n");

/* Now unwind the stack. I keep track of sp here (as well as fp), but for */
/* the moment I make no use of it.                                       */
    while (uwb->fp!=0)
    {   int nargs;
        char *name = 0;
        int *fp = (int *)uwb->fp;
        int *z = (int *)((fp[0] & 0x03fffffc) - 12);
/* Note that when I save pc in a STM instruction it points 12 beyond the */
/* instruction, not just 8!                                              */
/* If the word before the STM is itself STM sp!, {a1-a4} that shows      */
/* where I should find args, and suggests that there are >= 5.           */
        int *argp = fp+1;
        if (*(z-1) == 0xe92d000f) {
            nargs = 5;
        } else {
            int mask = *z & 0xffff;
/* Otherwise args were stored as part of the main STM. Find out where &  */
/* how many.                                                             */
            nargs = 0;
            while (mask != 0)
            {   argp--;
                if (mask & 0xf) ++nargs;
                mask ^= mask & (-mask);
            }
        }
/* Print args from the highest one downwards, in hex and decimal         */
        argp += nargs;
        while (nargs!=0)
        {   int v = *(--argp);
            fprintf(err, "  Arg%d: %#.8x %d", nargs--, v, v);
/* Indirect through addresses that might be legal...                     */
/* N.B. beware one day _codebase may be higher than base of data space   */
            if ((int)_codebase <= v && v < (int)top_of_stack &&
                (v & 0xfc000003) == 0)
            {   int *vp = (int *)v;
                fprintf(err, " -> [%#.8x %#.8x %#.8x %#.8x]",
                        vp[0], vp[1], vp[2], vp[3]);
            }
            fputc('\n', err);
        }
/* I search up to 10 words before the STM looking for the marker that    */
/* shows me where the function name is.                                  */
        {   int i;
            for (i=0; i<10; i++)
            {   int w = *--z;
                if ((w & 0xffff0000) == 0xff000000)
                {   name = (char *)z - (w & 0xffff);
                    break;
                }
            }
        }
        p_in(uwb);
        if (name != 0)
            fprintf(err, "function %s\n", name);
        else
            fprintf(err, "anonymous function\n");

        if (__rt_unwind(uwb) < 0) {
            fprintf(err, "\nstack corrupt\n");
            break;
        }
    }
    _exit(EXIT_FAILURE);
}

