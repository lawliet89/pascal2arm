/* stdlib.c: ANSI draft (X3J11 Oct 86) library, section 4.10 */
/* Copyright (C) Codemist Ltd, 1988                          */
/* Copyright (C) Advanced Risc Machines Ltd., 1991           */
/* version 0.02a */

/*
 * RCS $Revision: 1.5 $
 * Checkin $Date: 1995/02/01 11:35:24 $
 * Revising $Author: enevill $
 */

#include <stdlib.h>
#include <stddef.h>

#if defined _version_c || defined SHARED_C_LIBRARY

#include "externs.h"
#include "version.h"

const char *_clib_version(void)
{   return  "C Library vsn " __LIB_ISSUE " [" __DATE__ "]";
}

#endif

#if defined rand_c || defined SHARED_C_LIBRARY

/* The random-number generator that the world is expected to use */

static unsigned _random_number_seed[55] =
/* The values here are just those that would be put in this horrid
   array by a call to __srand(1). DO NOT CHANGE __srand() without
   making a corresponding change to these initial values.
*/
{   0x00000001, 0x66d78e85, 0xd5d38c09, 0x0a09d8f5, 0xbf1f87fb,
    0xcb8df767, 0xbdf70769, 0x503d1234, 0x7f4f84c8, 0x61de02a3,
    0xa7408dae, 0x7a24bde8, 0x5115a2ea, 0xbbe62e57, 0xf6d57fff,
    0x632a837a, 0x13861d77, 0xe19f2e7c, 0x695f5705, 0x87936b2e,
    0x50a19a6e, 0x728b0e94, 0xc5cc55ae, 0xb10a8ab1, 0x856f72d7,
    0xd0225c17, 0x51c4fda3, 0x89ed9861, 0xf1db829f, 0xbcfbc59d,
    0x83eec189, 0x6359b159, 0xcc505c30, 0x9cbc5ac9, 0x2fe230f9,
    0x39f65e42, 0x75157bd2, 0x40c158fb, 0x27eb9a3e, 0xc582a2d9,
    0x0569d6c2, 0xed8e30b3, 0x1083ddd2, 0x1f1da441, 0x5660e215,
    0x04f32fc5, 0xe18eef99, 0x4a593208, 0x5b7bed4c, 0x8102fc40,
    0x515341d9, 0xacff3dfa, 0x6d096cb5, 0x2bb3cc1d, 0x253d15ff
};

static int _random_j = 23, _random_k = 54;

int rand()
{
/* See Knuth vol 2 section 3.2.2 for a discussion of this random
   number generator.
*/
    unsigned int temp;
    temp = (_random_number_seed[_random_k] += _random_number_seed[_random_j]);
    if (--_random_j < 0) _random_j = 54, --_random_k;
    else if (--_random_k < 0) _random_k = 54;
    return (temp & 0x7fffffff);         /* result is a 31-bit value */
    /* It seems that it would not be possible, under ANSI rules, to */
    /* implement this as a 32-bit value. What a shame!              */
}

void srand(unsigned int seed)
{
/* This only allows you to put 32 bits of seed into the random sequence,
   but it is very improbable that you have any good source of randomness
   that good to start with anyway! A linear congruential generator
   started from the seed is used to expand from 32 to 32*55 bits.
*/
    int i;
    _random_j = 23;
    _random_k = 54;
    for (i = 0; i<55; i++)
    {   _random_number_seed[i] = seed + (seed >> 16);
/* This is not even a good way of setting the initial values.  For instance */
/* a better scheme would have r<n+1> = 7^4*r<n> mod (3^31-1).  Still I will */
/* leave this for now.                                                      */
        seed = 69069*seed + 1725307361;  /* computed modulo 2^32 */
    }
}

#endif

#if defined ANSI_rand_c || defined SHARED_C_LIBRARY

static unsigned long int next = 1;

int _ANSI_rand()     /* This is the ANSI suggested portable code */
{
    next = next * 1103515245 + 12345;
    return (unsigned int) (next >> 16) % 32768;
}

void _ANSI_srand(unsigned int seed)
{
    next = seed;
}

#endif

#if defined qsort_c || defined SHARED_C_LIBRARY

#ifdef INTERWORK
extern int __call_r2(const void *, const void *, int (*compar)(const void *, const void *));
#endif

/* Qsort is implemented using an explicit stack rather than C recursion  */
/* See Sedgewick (Algorithms, Addison Wesley, 1983) for discussion.      */

#define STACKSIZE 32
#define SUBDIVISION_LIMIT 10

#define exchange(p1, p2, size)                      \
{                                                   \
    int xsize = size;                               \
    if (((xsize | (int) p1) & 0x3)==0)              \
    {   do                                          \
        {   int temp = *(int *)p1;                  \
            *(int *)p1 = *(int *)p2;                \
            *(int *)p2 = temp;                      \
            p1 += 4;                                \
            p2 += 4;                                \
            xsize -= 4;;                            \
        } while (xsize != 0);                       \
    }                                               \
    else                                            \
    {   do                                          \
        {   char temp = *p1;                        \
            *p1++ = *p2;                            \
            *p2++ = temp;                           \
            xsize--;                                \
        } while (xsize != 0);                       \
    }                                               \
    p1 -= size;                                     \
    p2 -= size;                                     \
}

#define stack(p, n)                                 \
    (basestack[stackpointer] = (void *) (p),        \
     sizestack[stackpointer++] = (n))



static void partition_sort(void *base, size_t nmemb, size_t size,
                           int (*compar)(const void *, const void *))
{
    int stackpointer = 1;
    void *basestack[STACKSIZE];
    int sizestack[STACKSIZE];
/* The explicit stack that is used here is large enough for any array    */
/* that could exist on this computer - since I stack sub-intervals in a  */
/* careful order I can guarantee that the depth of stack needed is       */
/* bounded by log2(nmemb), so I can certainly handle arrays of up to     */
/* size 2**STACKSIZE, which is bigger than my address space.             */
/* I require that SUBDIVISION_LIMIT >= 2 so that all segments to be      */
/* partitioned have at least 3 items in them. This means that the first, */
/* middle and last items in a segment are distinct (although they may,   */
/* of course, have the same value).                                      */
    basestack[0] = base;
    sizestack[0] = nmemb;
    while (stackpointer!=0)
    {   char *b = (char *) basestack[--stackpointer];
        int n = sizestack[stackpointer];
/* The for loop here marks where we re-enter the code having refined an  */
/* interval.                                                             */
        for (;;)
        {   int halfn = (n >> 1) * size;  /* index of middle in the list */
            char *bmid = b + halfn;
            char *btop;
            if ((n & 1) == 0) halfn -= size;
            btop = bmid + halfn;
/* Sort first, middle and last items in the segment into order. Put the  */
/* smallest and largest at the start and end of the segment (where they  */
/* act as sentinel values in a conveniant way) and use the median of 3   */
/* as my pivot. This happens to ensure that sorted and inverse sorted    */
/* input gets pivoted neatly and costs n*log n operations, even though   */
/* there are STILL worst cases that take about n*n operations.           */
#ifdef INTERWORK
            if (__call_r2(b, bmid, compar) > 0) exchange(b, bmid, size);
            if (__call_r2(bmid, btop, compar) > 0)
            {   if (__call_r2(b, btop, compar) > 0) exchange(b, btop, size);
                exchange(bmid, btop, size);
            }
#else
            if (compar(b, bmid) > 0) exchange(b, bmid, size);
            if (compar(bmid, btop) > 0)
            {   if (compar(b, btop) > 0) exchange(b, btop, size);
                exchange(bmid, btop, size);
            }
#endif
/* Now I have the first and last elements in place & a useful pivot.     */
            {   char *l;
                char *r = btop - size;
                char *pivot;
                int s1, s2;
                exchange(bmid, r, size);
                l = b;
                pivot = r;
/* Sweep inwards partitioning elements on the basis of the pivot         */
                for (;;)
                {
#ifdef INTERWORK
                    do l += size; while (__call_r2(l, pivot, compar) < 0);
                    do r -= size; while (__call_r2(r, pivot, compar) > 0);
#else
                    do l += size; while (compar(l, pivot) < 0);
                    do r -= size; while (compar(r, pivot) > 0);
#endif
                    if (r <= l) break;
                    exchange(l, r, size);
                }
/* Move the pivot value to the middle of the array where it wants to be. */
                exchange(l, pivot, size);
/* s1 and s2 get the sizes of the sublists that I have partitioned my    */
/* data into. Note that s1 + s2 = n - 1 since the pivot element is now   */
/* in its correct place.                                                 */
                s1 = ((l - b)/ size) - 1;
                s2 = n - s1 - 1;
/* I am not going to try partitioning things that are smaller than some  */
/* fairly arbitrary small size (SUBDIVISION_LIMIT), and (only) if I will */
/* have to subdivide BOTH remaining intervals I push the larger of them  */
/* onto a stack for later consideration.                                 */
                if (s1 > s2)
/* Here the segment (b, s1) is the larger....                            */
                {   if (s2 > SUBDIVISION_LIMIT)
/* If the smaller segment needs subdividing then a fortiori the larger   */
/* one does, and needs stacking.                                         */
                    {   stack(b, s1);
                        b = l;
                        n = s2;
                    }
/* If just the larger segment needs dividing more I can loop.            */
                    else if (s1 > SUBDIVISION_LIMIT) n = s1;
/* If both segments are now small I break out of the loop.               */
                    else break;
                }
                else
/* Similar considerations apply if (l, s2) is the larger segment.        */
                {   if (s1 > SUBDIVISION_LIMIT)
                    {   stack(l, s2);
                        n = s1;
                    }
                    else if (s2 > SUBDIVISION_LIMIT)
                    {   b = l;
                        n = s2;
                    }
                    else break;
                }
            }
        }
    }
}

void qsort(void *base, size_t nmemb, size_t size,
           int (*compar)(const void *, const void *))
{
/* Sort an array (base) with nmemb items each of size bytes.             */
/* This uses a quicksort method, but one that is arranged to complete in */
/* about n*log(n) steps for sorted and inverse sorted inputs.            */
    char *b, *endp;
    if (size == 0) return;
    if (nmemb > SUBDIVISION_LIMIT)
        partition_sort(base, nmemb, size, compar);
/* Now I do an insertion sort on the array that is left over. This makes */
/* sense because the quicksort activity above has arranged that the      */
/* array is nearly sorted - at most segments of size SUBDIVISION_LIMIT   */
/* contain locally unsorted segments. In this case insertion sort goes   */
/* as fast as anything else. Doing things this way avoids the need for   */
/* a certain amount of partitioning/recursing overhead in the main body  */
/* of the quicksort procedure.                                           */
/* Note: some sorts of bugs in the above quicksort could be compensated  */
/* for here, leaving this entire procedure as just an insertion sort.    */
/* This would yield correct results but would be somewhat slow.          */
    b = (char *) base;
    endp = b + (nmemb-1) * size;
    while (b < endp)
    {   char *b1 = b;
        b += size;
/* Find out where to insert this item.                                   */
#ifdef INTERWORK
        while (b1>=(char *)base && __call_r2(b1, b, compar)>0) b1 -= size;
#else
        while (b1>=(char *)base && compar(b1, b)>0) b1 -= size;
#endif
        b1 += size;
/* The fact that I do not know how large the objects that are being      */
/* sorted are is horrible - I do an exchange here copying things one     */
/* word at a time or one byte at a time depending on the value of size.  */
        if (((size | (int) b) & 0x3)==0)
        {   int j;
            for (j=0; j<size; j+=4)
            {   char *bx = b + j;
/* Even when moving word by word I use (char *) pointers so as to avoid  */
/* muddles about pointer arithmetic. I cast to (int *) pointers when I   */
/* am about to dereference something.                                    */
                int save = *(int *)bx;
                char *by = bx - size;
                while (by>=b1)
                {   *(int *)(by + size) = *(int *)by;
                    by -= size;
                }
                *(int *)(by + size) = save;
            }
        }
        else
/* If size is not a multiple of 4 I behave with great caution and copy   */
/* byte by byte. I could, of course, copy by words up to the last 1, 2   */
/* or 3 bytes - I count that as too much effort for now.                 */
        {   int j;
            for (j=0; j<size; j++)
            {   char *bx = b + j;
                char save = *bx;
                char *by = bx - size;
                while (by>=b1)
                {   *(by + size) = *by;
                    by -= size;
                }
                *(by + size) = save;
            }
        }
    }
}

#endif

#if defined bsearch_c || defined SHARED_C_LIBRARY

#ifdef INTERWORK
extern int __call_r2(const void *, const void *, int (*compar)(const void *, const void *));
#endif

void *bsearch(const void *key, const void *base,
              size_t nmemb, size_t size,
              int (*compar)(const void *, const void *))
{
/* Binary search for given key in a sorted array (starting at base).     */
/* Comparisons are performed using the given function, and the array has */
/* nmemb items in it, each of size bytes.                                */
    int midn, c;
    void *midp;
    for (;;)
    {   if (nmemb==0) return NULL;      /* not found at all.             */
        else if (nmemb==1)
/* If there is only one item left in the array it is the only one that   */
/* needs to be looked at.                                                */
        {
#ifdef INTERWORK
            if (__call_r2(key, base, compar)==0) return (void *)base;
#else
            if (compar(key, base)==0) return (void *)base;
#endif
            else return NULL;
        }
        midn = nmemb>>1;                /* index of middle item          */
/* I have to cast bast to (char *) here so that the addition will be     */
/* performed using natural machine arithmetic.                           */
        midp = (char *)base + midn * size;
#ifdef INTERWORK
        c = __call_r2(key, midp, compar);
#else
        c = compar(key, midp);
#endif
        if (c==0) return midp;          /* item found (by accident)      */
        else if (c<0) nmemb = midn;     /* key is to left of midpoint    */
        else                            /* key is to the right           */
        {   base = (char *)midp + size; /* exclude the midpoint          */
            nmemb = nmemb - midn - 1;
        }
        continue;
    }
}

#endif

#if defined atexit_c || defined SHARED_C_LIBRARY

/*#include "hostsys.h"*/      /* for _terminateio(), and _exit() etc */
#include "interns.h"

#define EXIT_LIMIT 33

typedef void (*vprocp)(void);
static vprocp _exitvector[EXIT_LIMIT] = {};
       /* initialised so not in bss (or shared library trouble) */
static int number_of_exit_functions;

void __rt_exit_init()
{
    number_of_exit_functions = 0;
}

int atexit(vprocp func)
{
    if (number_of_exit_functions >= EXIT_LIMIT) return 1;    /* failure */
    _exitvector[number_of_exit_functions++] = func;
    return 0;                                                /* success */
}

#ifdef INTERWORK
extern void __call_r0(vprocp fn);
#endif

void __rt_call_exit_fns()
{
    while (number_of_exit_functions!=0) {
        vprocp fn = _exitvector[--number_of_exit_functions];
#ifdef INTERWORK
        __call_r0(fn);
#else
        fn();
#endif
    }
}

#endif

#if defined exit_c || defined SHARED_C_LIBRARY

#include "interns.h"

void exit(int n)
{
    __rt_lib_shutdown(1);
    __rt_exit(n);
}

#endif

#if defined abort_c || defined SHARED_C_LIBRARY

#include <signal.h>
#include "interns.h"

void abort()
{
    raise(SIGABRT);
    __rt_exit(1);
}

#endif

