struct S { int x, y; };
void escape(int *p);
int test(int a) { struct S s = {a, 0}; escape(&s.x); return s.y; }
