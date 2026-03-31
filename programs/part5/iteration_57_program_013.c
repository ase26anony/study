/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create loop-invariant values */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) int high_latency_op(int x, int y) {
    /* Division operation modeled with higher latency */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline)) int compute_loop(int* a, int* b, int* c, int n) {
    int sum = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = g_invariant1;
    int inv2 = g_invariant2;
    int inv3 = g_invariant3;
    
    /* Main loop with high register pressure */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + inv1;          /* Use invariant */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];        /* Array access with offset */
        int t5 = t4 * inv2;          /* Another invariant */
        int t6 = t5 - b[i+1];        /* Forward array access */
        int t7 = high_latency_op(t6, inv3);  /* Higher latency op */
        int t8 = t7 + c[i-1];
        int t9 = t8 * t0;
        int t10 = t9 - t2;
        int t11 = t10 + t4;
        int t12 = t11 * t6;
        int t13 = t12 - t8;
        int t14 = t13 + t10;
        int t15 = t14 * inv1;        /* Reuse invariant */
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 + a[i];
            int t17 = t16 * b[i];
            int t18 = high_latency_op(t17, 3);  /* Another high latency op */
            sum += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 - a[i];
            int t17 = t16 / 2;       /* Division with potential higher latency */
            int t18 = t17 * c[i];
            sum += t18;
        }
        
        /* Cross-iteration dependency through array */
        a[i] = t15 & 0xFF;  /* Create loop-carried dependency */
    }
    
    return sum;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        a[i] = (seed >> 16) & 0xFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    const int N = 1024;
    int* a = malloc(N * sizeof(int));
    int* b = malloc(N * sizeof(int));
    int* c = malloc(N * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic values */
    init_arrays(a, b, c, N);
    
    /* Call hot function multiple times to ensure it's compiled */
    int total_sum = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (iter * 3) % 17 + 5;
        g_invariant2 = (iter * 7) % 23 + 11;
        g_invariant3 = (iter * 11) % 29 + 17;
        
        total_sum += compute_loop(a, b, c, N);
    }
    
    printf("Result: %d\n", total_sum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
