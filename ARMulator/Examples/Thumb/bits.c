/* return the number of bits present in the unsigned integer n.          */
int bitcount(unsigned n)
{
    unsigned r;

    r = 0;
    while (n!=0) {
        r++;
        n ^= n & (-n);
    }
    return r;
}

/* If n is an exact power of two this returns the power, else -1         */
int power_of_two(unsigned n)
{
    unsigned s, w;

    if (n != (n&(-n))) return -1;
    s = 0;
    for (w=1; w!=0; w = w<<1)
        if (w==n) return(s);
        else s++;
    return -1;
}

unsigned reverse_bits(unsigned n)
{
    unsigned r, old_r;

    r = 1;
    do {
        old_r = r;
        r <<= 1;
        if (n & 1) r++;
        n >>= 1;
    } while (r > old_r);
    return r;
}
