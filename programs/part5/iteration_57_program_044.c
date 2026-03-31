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
    /* Force higher latency operation */
    return (x % y) + (x / (y + 1));
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
        int t7 = t6 / (inv3 + 1);     /* Division for higher latency */
        int t8 = t7 + slow_operation(t6, inv1); /* High latency call */
        int t9 = t8 * t3;
        int t10 = t9 - t1;
        int t11 = t10 + a[i];
        int t12 = t11 * b[i-1];
        int t13 = t12 - t4;
        int t14 = t13 + c[i+1];
        int t15 = t14 * inv1;
        int t16 = t15 - t7;
        int t17 = t16 + t11;
        int t18 = t17 * inv2;
        int t19 = t18 - t13;
        int t20 = t19 + slow_operation(t18, inv3); /* Another high latency call */
        
        /* Conditional to create multiple basic blocks */
        if (t20 & 1) {
            /* Path 1: More computations */
            int t21 = t20 * 3;
            int t22 = t21 + t5;
            int t23 = t22 - t9;
            int t24 = t23 / 5;
            result += t24 + a[i];
        } else {
            /* Path 2: Different computations */
            int t21 = t20 / 2;
            int t22 = t21 + t10;
            int t23 = t22 * t15;
            int t24 = t23 - t19;
            result += t24 + b[i];
        }
        
        /* Cross-iteration dependency */
        a[i] = t20 & 0xFF;  /* Modify array for loop-carried dependency */
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        a[i] = seed % 1000;
        b[i] = (seed * 1664525 + 1013904223) & 0x7FFFFFFF;
        c[i] = seed % 500;
    }
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
    
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 2) & 0xFF;
        g_invariant3 = (g_invariant3 * 7 + 3) & 0xFF;
        
        total_result += compute_loop(a, b, c, SIZE);
        
        /* Re-initialize arrays occasionally to vary data */
        if (iter % 10 == 0) {
            init_arrays(a, b, c, SIZE);
        }
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
