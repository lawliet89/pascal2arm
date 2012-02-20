/* This header file contains various definitions for the 64 bit mul
 * instructions. It must be included in any C source wishing to use these
 * functions.
 */
typedef struct Int64 { int lo, hi; } Int64;
typedef struct Unsigned64 { unsigned lo, hi; } Unsigned64;

/* Return 64 bit signed result of 'a' * 'b' */
extern __value_in_regs Int64 smull(int a, int b);

/* Return 64 bit signed result of 'a' * 'b' + 'acc' */
extern __value_in_regs Int64 smlal(Int64 acc, int a, int b);

/* Return 64 bit unsigned result of 'a' * 'b' */
extern __value_in_regs Unsigned64 umull(unsigned a, unsigned b);

/* Return 64 bit unsigned result of 'a' * 'b' + 'acc' */
extern __value_in_regs Unsigned64 umlal(Unsigned64 acc, unsigned a, unsigned b);
