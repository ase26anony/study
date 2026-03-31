/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop invariants */
int g_inv1 = 7;
int g_inv2 = 13;
int g_inv3 = 19;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_hot_loop(int* arr1, int* arr2, int* arr3, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Start chain with loop-invariant values (creates potential moves) */
        t0 = g_inv1 * arr1[i];      /* Use invariant */
        t1 = t0 + g_inv2;           /* Another invariant */
        
        /* Chain of dependent operations */
        t2 = t1 * arr2[i];
        t3 = t2 - arr3[i];
        t4 = t3 + arr1[i-1];        /* Array access with offset */
        t5 = t4 * arr2[i+1];        /* Another offset access */
        t6 = t5 / g_inv3;           /* Division with invariant (higher latency) */
        
        /* More dependent operations */
        t7 = t6 + arr3[i-1];
        t8 = t7 * t1;
        t9 = t8 - t3;
        t10 = t9 + t5;
        
        /* Introduce conditional to create multiple basic blocks */
        if (t10 & 1) {
            /* Path 1: More computations */
            t11 = t10 * 3;
            t12 = t11 + arr1[i];
            t13 = t12 - t6;
            t14 = t13 * 2;
            t15 = t14 + g_inv1;     /* Use invariant again */
        } else {
            /* Path 2: Different computations but still dependent */
            t11 = t10 / 2;          /* Another division */
            t12 = t11 + t4;
            t13 = t12 * arr2[i];
            t14 = t13 - t8;
            t15 = t14 + g_inv2;     /* Use invariant */
        }
        
        /* Continue chain after conditional */
        t16 = t15 * t7;
        t17 = t16 - t9;
        t18 = t17 + arr3[i+1];      /* Another offset access */
        t19 = t18 / 4;              /* Final operation */
        
        /* Accumulate result */
        result += t19;
    }
    
    return result;
}

/* Helper to initialize arrays with deterministic values */
void init_arrays(int* a, int* b, int* c, int n) {
    unsigned int seed = 12345;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    long long total = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_inv1 = (g_inv1 * 13 + 7) & 0xFF;
        g_inv2 = (g_inv2 * 17 + 11) & 0xFF;
        g_inv3 = (g_inv3 * 19 + 13) & 0xFF;
        
        total += compute_hot_loop(a, b, c, SIZE);
    }
    
    printf("Result: %lld\n", total);
    return 0;
}
