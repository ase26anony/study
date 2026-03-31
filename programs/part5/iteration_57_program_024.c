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
    /* Division creates higher latency */
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
        int t7 = high_latency_op(t6, inv3); /* Higher latency op */
        int t8 = t7 + c[i-1];
        int t9 = t8 * t0;
        int t10 = t9 - t2;
        int t11 = t10 + t4;
        int t12 = t11 * t6;
        int t13 = t12 - t8;
        int t14 = t13 + t10;
        int t15 = t14 * inv1;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 + a[i];
            int t17 = t16 * b[i];
            int t18 = high_latency_op(t17, 3);
            result += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 - a[i];
            int t17 = t16 / 2;        /* Division for latency */
            int t18 = t17 * c[i];
            result += t18;
        }
        
        /* Cross-iteration dependency */
        a[i] = t15 & 0xFF;  /* Modify array for next iteration */
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
unsigned int simple_rand(unsigned int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return *seed;
}

int main() {
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call hot function multiple times to ensure execution */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariant globals slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) % 29;
        g_invariant2 = (g_invariant2 * 5 + 2) % 31;
        g_invariant3 = (g_invariant3 * 7 + 3) % 37;
        
        total_result += compute_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
