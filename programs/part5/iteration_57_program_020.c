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
    return (x % y) != 0 ? x / (y + 1) : x;
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
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
        
        /* Start chain with array accesses and invariant mixing */
        t0 = a[i] + inv1;                    /* Mix invariant */
        t1 = b[i] * t0;
        t2 = c[i] - t1;
        t3 = a[i-1] + t2;                    /* Use offset access */
        t4 = b[i+1] * t3;                    /* Another offset access */
        t5 = t4 / (inv2 + 1);                /* Division with invariant */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More operations with dependencies */
            t6 = t5 * inv3;
            t7 = a[i] + t6;
            t8 = b[i] * t7;
            t9 = high_latency_op(t8, inv1);  /* Higher latency call */
            t10 = t9 - c[i];
        } else {
            /* Path 2: Different but still dependent chain */
            t6 = t5 + inv1;
            t7 = c[i-1] * t6;                /* Different offset */
            t8 = high_latency_op(t7, inv2);  /* Higher latency */
            t9 = a[i+1] + t8;
            t10 = t9 * inv3;
        }
        
        /* Continue dependency chain after merge */
        t11 = t10 + a[i];
        t12 = t11 * b[i];
        t13 = high_latency_op(t12, t11);     /* Another high latency op */
        t14 = t13 - c[i];
        t15 = t14 * inv2;
        
        /* Accumulate result preventing dead code elimination */
        result += t15;
    }
    
    return result;
}

/* Simple deterministic pseudo-random generator */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        a[i] = (seed >> 16) & 0xFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int total_result = 0;
    
    /* Initialize arrays with deterministic values */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled as hot */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 1) & 0xFF;
        
        total_result += compute_hot_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
