void block_copy(void *dest, void *source, int n)
{
    int *dest_ip, *source_ip;

    dest_ip = (int *)dest;      /* Move function arguments into integer  */
    source_ip = (int *)source;  /* integer for use in the following loop */
    while (n > 0) {
        *dest_ip++ = *source_ip++;
        n -= 4;
    }
}
