// 1. Loop Vectorization: Memory Dependency (Backward)
void loop_dependency(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];
    }
}

// 2. Inlining: Forced No-Inline
__attribute__((noinline))
int square(int x) {
    return x * x;
}

int use_square(int x) {
    return square(x) + square(x + 1);
}

// 3. Loop Vectorization: Control Flow (Early Exit)
int loop_with_exit(int *a, int n, int threshold) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) break;
        sum += a[i];
    }
    return sum;
}

// 4. SLP Vectorizer: Non-contiguous / Complex Stores
void slp_missed(float *a, float *b, float *c) {
    a[0] = b[0] + c[0];
    a[2] = b[1] + c[1]; // Gap at index 1
    a[4] = b[2] + c[2]; // Gap at index 3
}

// 5. SROA: Escaping Pointer
void escape(int *p);
void sroa_fail() {
    int x = 42;
    escape(&x); // Address taken prevents SROA/mem2reg
}
