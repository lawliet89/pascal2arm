typedef struct List {
    struct List *next;
    void *data;
} List;

int length(List *l)
{
    int n;

    n = 0;
    while (l) {
        l = l->next;
        n++;
    }
    return n;
}

List *reverse(List *l)
{
    List *new_l, *t;

    new_l = 0;
    while (l) {
        t = l->next;
        l->next = new_l;
        new_l = l;
        l = t;
    }
    return new_l;
}

List *conc(List *a, List *b)
{
    List *l, **l_p, *t;

    l = a;
    l_p = &l;
    while (t = *l_p) l_p = &(t->next);
    *l_p = b;
    return a;
}
