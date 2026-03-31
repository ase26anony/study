/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop invariants */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division modeled with higher latency */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    int invariant_local = g_invariant1 + g_invariant2;
    
    /* Main loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0 = a[i];
        int t1 = b[i] + invariant_local;  /* Loop-invariant use */
        int t2 = t0 * t1;
        int t3 = c[i] - g_invariant3;     /* Another invariant */
        int t4 = t2 + t3;
        int t5 = high_latency_op(t4, 17); /* Higher latency operation */
        int t6 = a[i-1] + t5;
        int t7 = b[i+1] * t6;
        int t8 = c[i] + t7;
        int t9 = t8 - a[i+1];
        int t10 = t9 * g_invariant2;      /* Another invariant use */
        int t11 = t10 + b[i-1];
        int t12 = t11 / 3;                /* Potential higher latency */
        int t13 = t12 + c[i+1];
        int t14 = t13 - a[i];
        int t15 = t14 * 5;
        
        /* Conditional creating multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 + high_latency_op(t15, 3);
            int t17 = t16 * a[i];
            int t18 = t17 - b[i];
            result += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 - high_latency_op(t15, 5);
            int t17 = t16 / 2;
            int t18 = t17 + c[i];
            result += t18;
        }
        
        /* Cross-iteration dependency */
        a[i] = t15 & 0xFF;  /* Creates loop-carried dependency */
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
    int a[SIZE], b[SIZE], c[SIZE];
    int total_result = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 2) & 0xFF;
        
        total_result += compute_hot_loop(a, b, c, SIZE);
        
        /* Re-initialize arrays every few iterations */
        if (iter % 10 == 9) {
            init_arrays(a, b, c, SIZE);
        }
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
