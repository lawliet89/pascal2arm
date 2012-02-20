#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N               1000

extern void sort(char **strings, int n);

static void randomise(char *strings[], int n)
{
    int i;
    int v;
    char *t;

    for (i = 0; i < N; i++) {
        v = rand() % N;
        t = strings[v];
        strings[v] = strings[i];
        strings[i] = t;
    }
}

int main(void)
{
    char *strings[N];
    char buffer[1000*(3+1)];
    char *p;
    int i;

    p = buffer;
    for (i = 0; i < 1000; i++) {
        sprintf(p, "%03d", i);
        strings[i] = p;
        p += 3+1;
    }
    randomise(strings, N);
    sort(strings, N);
}
