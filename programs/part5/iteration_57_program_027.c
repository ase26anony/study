/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop invariants */
int g_inv1 = 7;
int g_inv2 = 13;
int g_inv3 = 19;

/* Hot function with complex loop body */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + g_inv1;          /* Loop invariant use */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + g_inv2;          /* Another invariant */
        int t5 = t4 * a[i-1];          /* Array access with offset */
        int t6 = t5 / g_inv3;          /* Division with invariant (higher latency) */
        int t7 = t6 + b[i+1];          /* Forward array access */
        int t8 = t7 * t0;              /* Reuse earlier value */
        int t9 = t8 - c[i-1];          /* Backward array access */
        int t10 = t9 + a[i+1];
        int t11 = t10 * t3;
        int t12 = t11 / 3;             /* Another division */
        int t13 = t12 + t6;
        int t14 = t13 * t9;
        int t15 = t14 - t2;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 * g_inv1;
            int t17 = t16 + t8;
            int t18 = t17 / 5;         /* Division */
            int t19 = t18 - t11;
            result += t19;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 + g_inv2;
            int t17 = t16 * t13;
            int t18 = t17 - t4;
            int t19 = t18 / 7;         /* Division */
            result += t19;
        }
        
        /* Additional dependent operations outside condition */
        int t20 = result * 2;
        int t21 = t20 + t15;
        int t22 = t21 / 11;            /* Division */
        result = t22 + i;
    }
    
    return result;
}

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold))
int slow_operation(int x, int y) {
    /* Complex operation that won't be inlined */
    for (int i = 0; i < 3; i++) {
        x = (x * y) + i;
    }
    return x;
}

/* Another hot function with mixed operations */
__attribute__((hot, noinline))
int compute_with_slow_op(int* a, int* b, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Chain of dependencies */
        int v0 = a[i];
        int v1 = v0 + b[i];
        int v2 = v1 * g_inv1;
        int v3 = slow_operation(v2, g_inv2);  /* Function call */
        int v4 = v3 - a[(i + 1) % n];
        int v5 = v4 / g_inv3;                 /* Division */
        int v6 = v5 + b[(i + 2) % n];
        int v7 = v6 * v1;
        
        /* Conditional with both paths having computations */
        if (v7 > 0) {
            int v8 = v7 + g_inv1;
            int v9 = v8 * 3;
            sum += v9;
        } else {
            int v8 = v7 - g_inv2;
            int v9 = v8 / 2;                  /* Division */
            sum += v9;
        }
    }
    
    return sum;
}

/* Simple PRNG for deterministic values */
static unsigned int seed = 12345;
static inline int rand_int(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

int main(void) {
    /* Initialize arrays with deterministic pseudo-random values */
    int a[SIZE], b[SIZE], c[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand_int() % 1000;
        b[i] = rand_int() % 1000;
        c[i] = rand_int() % 1000;
    }
    
    /* Call hot functions multiple times to ensure execution */
    int total = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_inv1 = (g_inv1 * 3 + 1) % 100;
        g_inv2 = (g_inv2 * 5 + 2) % 100;
        g_inv3 = (g_inv3 * 7 + 3) % 100;
        
        /* Call both hot functions */
        total += compute_loop(a, b, c, SIZE);
        total += compute_with_slow_op(a, b, SIZE);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
