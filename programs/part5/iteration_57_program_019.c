/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Global variables to create loop-invariant values */
int g_inv1 = 7;
int g_inv2 = 13;
int g_inv3 = 19;

/* Hot function with complex loop body */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Start chain with loop-invariant values (creates potential moves) */
        t0 = g_inv1 * a[i];          /* Use invariant */
        t1 = t0 + b[i-1];            /* Array access with offset */
        t2 = t1 * g_inv2;            /* Another invariant */
        t3 = t2 - c[i+1];            /* Forward array access */
        t4 = t3 / g_inv3;            /* Division with invariant (higher latency) */
        
        /* Continue dependency chain */
        t5 = t4 + a[i] * b[i];
        t6 = t5 * t4 - t3;
        t7 = t6 / 3;                 /* Division operation */
        t8 = t7 + t2 * t1;
        t9 = t8 - t0 / 2;
        
        /* More operations to extend live ranges */
        t10 = t9 * a[i-1] + b[i];
        t11 = t10 / 5;               /* Another division */
        t12 = t11 + c[i] * t9;
        t13 = t12 - t8 / 7;
        t14 = t13 * t7 + t6;
        t15 = t14 / 11;              /* Division */
        t16 = t15 + t5 * t4;
        t17 = t16 - t3 / 13;
        t18 = t17 * t2 + t1;
        t19 = t18 / 17;              /* Division */
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* True path with computations */
            t19 = t19 * 3 + 1;
            result += t19 * a[i];
        } else {
            /* False path with different computations */
            t19 = t19 / 2;
            result += t19 * b[i];
        }
        
        /* Cross-iteration dependency with array */
        a[i] = t19 + result % 256;
    }
    
    return result;
}

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold))
int high_latency_op(int x, int y) {
    /* Complex operation that won't be inlined */
    for (int i = 0; i < 3; i++) {
        x = (x * y) % 9973;
        y = (y * 1664525 + 1013904223) % 9973;
    }
    return x ^ y;
}

/* Another hot function with mixed operations */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int result = 0;
    int invariant = high_latency_op(g_inv1, g_inv2);
    
    for (int i = 0; i < n; i++) {
        /* Complex dependency chain with invariant */
        int t0 = a[i] + invariant;
        int t1 = t0 * b[i % n];
        int t2 = t1 - c[(i + 1) % n];
        int t3 = high_latency_op(t2, t1);  /* High latency call */
        int t4 = t3 / g_inv3;              /* Division */
        
        int t5 = t4 + a[(i + 2) % n];
        int t6 = t5 * t4 - t3;
        int t7 = high_latency_op(t6, 31);  /* Another high latency call */
        
        /* Conditional with computations in both paths */
        if (t7 > 0) {
            t7 = t7 * 2 + a[i];
            result += t7;
        } else {
            t7 = t7 / 2 - b[i];
            result -= t7;
        }
        
        /* Store with cross-iteration dependency */
        c[i] = t7 + result % 512;
    }
    
    return result;
}

/* Simple PRNG for deterministic values */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

int main() {
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand() % 1000;
        b[i] = simple_rand() % 1000;
        c[i] = simple_rand() % 1000;
    }
    
    int total_result = 0;
    
    /* Call hot functions multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        total_result += compute_loop(a, b, c, SIZE);
        total_result += compute_loop2(a, b, c, SIZE);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) % 1000;
            b[i] = (b[i] * 3) % 1000;
        }
    }
    
    printf("Result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
