#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

typedef union
{
    u_char c[4];
    u_int  i;
} Swap;

int main(void)
{
    u_char buffer[512];
    u_int nbytes;
    u_int i, nout;
    Swap swapit;

    while ((nbytes = read(fileno(stdin), buffer, sizeof(buffer))) != 0)
    {
        if (nbytes < 0)
        {
            perror("read");
            return 1;
        }

        if ((nbytes % 4) != 0)
        {
            fprintf(stderr, "stdin not a integer number of words\n");
            return 1;
        }

        for ( i = 0; i < nbytes; i += 4)
        {
            swapit.c[3] = buffer[i + 0];
            swapit.c[2] = buffer[i + 1];
            swapit.c[1] = buffer[i + 2];
            swapit.c[0] = buffer[i + 3];

            *((u_int *)(buffer + i)) = swapit.i;
        }

        if ((nout = write(fileno(stdout), buffer, nbytes)) != nbytes)
        {
            if (nbytes < 0)
                perror("write");
            else
                fprintf(stderr, "write: asked for %d, got %d\n", nbytes, nout);

            return 1;
        }
    }

    return 0;
}

/* EOF wordswap.c */
