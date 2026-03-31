/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop invariants */
int global_invariant1 = 12345;
int global_invariant2 = 67890;
int global_invariant3 = 54321;

/* Hot function with complex loop body */
__attribute__((hot, noinline))
int compute_hot_loop(int* restrict a, int* restrict b, int* restrict c, 
                     int* restrict d, int n, int seed) {
    int result = seed;
    
    /* Main loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating long dependency chain */
        int t0 = a[i] + global_invariant1;  /* Loop invariant use */
        int t1 = t0 * b[i];
        int t2 = t1 - global_invariant2;    /* Another invariant */
        int t3 = t2 + c[i];
        int t4 = t3 * a[i-1];               /* Array access with offset */
        int t5 = t4 / (global_invariant3 | 1); /* Division for higher latency */
        int t6 = t5 + b[i+1];               /* Forward offset */
        int t7 = t6 * t0;                   /* Cross-iteration dependency potential */
        int t8 = t7 - c[i-1];               /* Backward offset */
        int t9 = t8 * global_invariant1;    /* Reuse invariant */
        int t10 = t9 + t2;
        int t11 = t10 * t5;
        int t12 = t11 - t8;
        int t13 = t12 / (t3 | 1);           /* Another division */
        int t14 = t13 + t6;
        int t15 = t14 * t11;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 * global_invariant2;
            int t17 = t16 + a[i];
            int t18 = t17 - t10;
            int t19 = t18 * b[i];
            result += t19;
            
            /* Additional operations in this path */
            int t20 = t19 % (global_invariant3 | 1); /* Modulo for latency */
            int t21 = t20 + t14;
            d[i] = t21;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 + global_invariant3;
            int t17 = t16 * c[i];
            int t18 = t17 - t12;
            int t19 = t18 / (global_invariant1 | 1);
            result -= t19;
            
            /* More operations in else path */
            int t20 = t19 * t13;
            int t21 = t20 + t8;
            d[i] = t21;
        }
        
        /* Continue dependency chain after conditional */
        int t22 = d[i] * t15;
        int t23 = t22 + global_invariant1;
        int t24 = t23 - t13;
        int t25 = t24 * t22;
        
        /* Use result to create loop-carried dependency */
        result ^= t25;
    }
    
    return result;
}

/* Simple deterministic PRNG for array initialization */
static inline int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main() {
    /* Allocate and initialize arrays */
    int a[SIZE], b[SIZE], c[SIZE], d[SIZE];
    int seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
        d[i] = 0;
    }
    
    /* Call hot function multiple times to ensure it's compiled */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        int loop_seed = simple_rand(&seed) % 100;
        total_result += compute_hot_loop(a, b, c, d, SIZE, loop_seed);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) % 1000;
            b[i] = (b[i] * 3) % 1000;
        }
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
