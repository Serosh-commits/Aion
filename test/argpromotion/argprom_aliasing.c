static int test(int *p, int *q) { return *p + *q; }
int caller(int *a) { return test(a, a); }
