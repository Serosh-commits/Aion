struct S { volatile int x; int y; };
int test(int a) { struct S s = {a, 0}; return s.x; }
