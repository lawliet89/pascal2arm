#include <string.h>

void sort(char *strings[], int n)
{
    int h, i, j;
    char *v;

    strings--;        /* Make array 1 origin */
    h = 1;
    do {h = h * 3 + 1;} while (h <= n);
    do {
        h = h / 3;
        for (i = h + 1; i <= n; i++) {
            v = strings[i];
            j = i;
            while (j > h && strcmp(strings[j-h], v) > 0) {
                strings[j] = strings[j-h];
                j = j-h;
            }
            strings[j] = v;
        }
    } while (h > 1);
}
