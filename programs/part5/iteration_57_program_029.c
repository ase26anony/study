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
__attribute__((noinline, cold)) 
int slow_operation(int x, int y) {
    /* Division operation with higher latency */
    return (x % y) + (y % x);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating long dependency chain */
        int t0 = a[i];
        int t1 = t0 + g_invariant1;           /* Use loop-invariant */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];                 /* Array access with offset */
        int t5 = t4 * g_invariant2;           /* Another loop-invariant */
        int t6 = t5 - b[i+1];                 /* Forward array access */
        
        /* Operation with potentially higher latency */
        int t7 = slow_operation(t6, g_invariant3);
        
        int t8 = t7 + c[i-1];
        int t9 = t8 * t0;
        int t10 = t9 - t2;
        int t11 = t10 + t4;
        int t12 = t11 * t6;
        int t13 = t12 - t8;
        int t14 = t13 + t10;
        int t15 = t14 * t12;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 + g_invariant1;
            int t17 = t16 * t13;
            int t18 = t17 - a[i];
            int t19 = t18 + b[i];
            result += t19;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 - g_invariant2;
            int t17 = t16 / 3;                /* Division adds latency */
            int t18 = t17 * c[i];
            int t19 = t18 + t14;
            result += t19;
        }
        
        /* Cross-iteration dependency */
        a[i] = t15 & 0xFF;  /* Modify array for next iteration */
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
unsigned int simple_rand(unsigned int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main() {
    /* Initialize arrays with deterministic pseudo-random values */
    int a[SIZE], b[SIZE], c[SIZE];
    unsigned int seed = 42;
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call hot function multiple times to ensure execution */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariant slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) % 31;
        
        total_result += compute_loop(a, b, c, SIZE);
        
        /* Rotate arrays to change data pattern */
        int temp = a[0];
        for (int i = 0; i < SIZE - 1; i++) {
            a[i] = a[i + 1];
        }
        a[SIZE - 1] = temp;
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
