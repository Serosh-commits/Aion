__attribute__((noinline)) int big(int x) { return x * 2 + 1; }
int caller(int x) { return big(x); }
