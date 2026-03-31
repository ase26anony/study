/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop-invariant values */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) int slow_operation(int x, int y) {
    /* Division operation modeled with higher latency */
    return (x % y) != 0 ? x / (y + 1) : x;
}

/* Hot function containing the target loop */
__attribute__((hot, noinline)) int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
        
        /* Start chain with loop-invariant values (creates potential moves) */
        t0 = g_invariant1 + i;
        t1 = t0 * g_invariant2;
        
        /* Array accesses with index variations */
        t2 = a[i] + t1;
        t3 = b[i-1] * t2;
        t4 = c[i+1] - t3;
        
        /* Mix operations - some with higher latency */
        t5 = slow_operation(t4, g_invariant3);
        t6 = t5 & 0xFF;
        t7 = t6 * a[i];
        
        /* Conditional creating multiple basic blocks */
        if (t7 & 1) {
            /* Path 1: More computations */
            t8 = t7 + b[i];
            t9 = t8 * 3;
            t10 = t9 - c[i];
            t11 = t10 >> 2;
            t12 = t11 * g_invariant1;
        } else {
            /* Path 2: Different computations but still dependent */
            t8 = t7 - b[i];
            t9 = t8 / 2;  /* Division - higher latency */
            t10 = t9 + c[i];
            t11 = t10 << 1;
            t12 = t11 % g_invariant2;  /* Modulo - higher latency */
        }
        
        /* Rejoin and continue dependency chain */
        t13 = t12 + a[i+1];
        t14 = t13 * t5;
        t15 = t14 - b[i-1];
        
        /* Accumulate result */
        result ^= t15;
        
        /* Cross-iteration dependency through array */
        a[i] = (t15 + result) & 0x7F;
    }
    
    return result;
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
    int arrays_a[SIZE];
    int arrays_b[SIZE];
    int arrays_c[SIZE];
    
    /* Initialize arrays */
    init_arrays(arrays_a, arrays_b, arrays_c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled as hot */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariant slightly each iteration to prevent constant propagation */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        
        total_result += compute_hot_loop(arrays_a, arrays_b, arrays_c, SIZE);
        
        /* Re-initialize arrays every few iterations */
        if (iter % 10 == 9) {
            init_arrays(arrays_a, arrays_b, arrays_c, SIZE);
        }
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
