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
    /* Division operation with higher latency */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = g_invariant1;
    int inv2 = g_invariant2;
    int inv3 = g_invariant3;
    
    /* Main loop with high register pressure */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + inv1;           /* Use invariant */
        int t2 = b[i] * t1;
        int t3 = t2 - c[i];
        int t4 = t3 * inv2;           /* Use another invariant */
        int t5 = t4 + a[i-1];         /* Array access with offset */
        int t6 = t5 - b[i+1];         /* Another offset access */
        int t7 = t6 * inv3;           /* Third invariant */
        
        /* High latency operation in the chain */
        int t8 = high_latency_op(t7, t1);
        
        int t9 = t8 + c[i-1];
        int t10 = t9 * a[i+1];
        int t11 = t10 - t2;
        int t12 = t11 + t4;
        int t13 = t12 * t6;
        int t14 = t13 - t8;
        int t15 = t14 + t10;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 * 3;
            int t17 = t16 + inv1;
            int t18 = high_latency_op(t17, 5);
            result += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 / 2;
            int t17 = t16 - inv2;
            int t18 = t17 * t3;
            result += t18;
        }
        
        /* Cross-iteration dependency through array */
        a[i] = t15 & 0xFF;
    }
    
    return result;
}

/* Simple deterministic random number generator */
int simple_rand(int seed) {
    return (seed * 1103515245 + 12345) & 0x7FFFFFFF;
}

int main() {
    /* Initialize arrays with deterministic values */
    int a[SIZE], b[SIZE], c[SIZE];
    
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = simple_rand(seed);
        a[i] = seed % 100;
        seed = simple_rand(seed);
        b[i] = seed % 100;
        seed = simple_rand(seed);
        c[i] = seed % 100;
    }
    
    /* Call hot function multiple times to ensure execution */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3) % 17;
        g_invariant2 = (g_invariant2 * 5) % 19;
        g_invariant3 = (g_invariant3 * 7) % 23;
        
        total_result += compute_hot_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
