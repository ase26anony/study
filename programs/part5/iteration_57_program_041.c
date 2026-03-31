/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

/* Global variables to create loop-invariant values */
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
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = g_invariant1;
    int inv2 = g_invariant2;
    int inv3 = g_invariant3;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + inv1;           /* Use invariant */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];         /* Array access with offset */
        int t5 = t4 * inv2;           /* Another invariant */
        int t6 = t5 - b[i+1];         /* Forward array access */
        int t7 = high_latency_op(t6, inv3); /* High latency op with invariant */
        int t8 = t7 + c[i-1];
        int t9 = t8 * t0;
        int t10 = t9 - t2;
        int t11 = t10 + a[i+1];
        int t12 = t11 * t4;
        int t13 = t12 - t7;
        int t14 = t13 + b[i];
        int t15 = t14 * t9;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 + inv1;
            int t17 = t16 * t3;
            int t18 = high_latency_op(t17, 5); /* Another high latency op */
            result += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 - inv2;
            int t17 = t16 / 3;        /* Division */
            int t18 = t17 * t8;
            result += t18;
        }
        
        /* Additional dependent chain outside condition */
        int t19 = t15 + result;
        int t20 = t19 * inv1;
        int t21 = high_latency_op(t20, 11);
        result ^= t21;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
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
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 1) & 0xFF;
        g_invariant3 = (g_invariant3 * 7 + 1) & 0xFF;
        
        total_result += compute_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
