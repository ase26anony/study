/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop-invariant values */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Force a division which typically has higher latency */
    return (x % y) + (y % (x | 1));
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = g_invariant1;
    int inv2 = g_invariant2;
    int inv3 = g_invariant3;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Chain of dependent operations with array accesses */
        t0 = a[i] + inv1;                    /* Use invariant */
        t1 = t0 * b[i];
        t2 = t1 - c[i];
        t3 = t2 + a[i-1];                    /* Different array offset */
        t4 = t3 * b[i+1];                    /* Another offset */
        t5 = t4 - inv2;                      /* Another invariant */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More dependent operations */
            t6 = t5 * inv3;                  /* Another invariant */
            t7 = high_latency_op(t6, 17);    /* Higher latency op */
            t8 = t7 + c[i-1];
            t9 = t8 * a[i];
            t10 = t9 - b[i];
            t11 = t10 + t5;
            t12 = t11 * 3;
            result += t12;
        } else {
            /* Path 2: Alternative dependent chain */
            t13 = t5 / 3;                    /* Division for latency */
            t14 = t13 + a[i+1];
            t15 = t14 * b[i-1];
            t16 = high_latency_op(t15, 23);  /* Higher latency op */
            t17 = t16 - c[i];
            t18 = t17 * 5;
            t19 = t18 + t5;
            result += t19;
        }
        
        /* Cross-iteration dependency */
        a[i] = (a[i] + result) & 0xFF;       /* Modify array for next iteration */
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        a[i] = seed % 100;
        b[i] = (seed >> 8) % 100;
        c[i] = (seed >> 16) % 100;
    }
}

int main() {
    int arrays_a[SIZE], arrays_b[SIZE], arrays_c[SIZE];
    int total_result = 0;
    
    /* Initialize arrays */
    init_arrays(arrays_a, arrays_b, arrays_c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 2) & 0xFF;
        g_invariant3 = (g_invariant3 * 7 + 3) & 0xFF;
        
        total_result += compute_hot_loop(arrays_a, arrays_b, arrays_c, SIZE);
        
        /* Re-initialize arrays periodically to avoid overflow patterns */
        if (iter % 10 == 9) {
            init_arrays(arrays_a, arrays_b, arrays_c, SIZE);
        }
    }
    
    printf("Final result: %d\n", total_result);
    return 0;
}
