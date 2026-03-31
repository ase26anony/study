/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

/* Global variables to create loop-invariant values */
int g_inv1 = 17;
int g_inv2 = 23;
int g_inv3 = 29;

/* Hot function with complex loop body */
__attribute__((hot, noinline))
int compute_loop(int* restrict a, int* restrict b, int* restrict c, int n) {
    int sum = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 2; i < n - 2; i++) {
        /* Declare many temporaries to increase register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Start chain with loop-invariant values (creates potential moves) */
        t0 = g_inv1 * a[i];          /* Use invariant */
        t1 = t0 + b[i-1];            /* Dependency chain */
        t2 = t1 * g_inv2;            /* Another invariant */
        t3 = t2 - c[i+1];
        t4 = t3 / 7;                 /* Division - higher latency */
        t5 = t4 + a[i-2];
        t6 = t5 * b[i];
        t7 = t6 - g_inv3;            /* Another invariant */
        t8 = t7 + c[i-1];
        t9 = t8 / 3;                 /* Another division */
        
        /* More operations to extend dependency chain */
        t10 = t9 * a[i+1];
        t11 = t10 + b[i+2];
        t12 = t11 * t0;              /* Cross-dependency */
        t13 = t12 - t4;
        t14 = t13 / 5;
        t15 = t14 + t7;
        t16 = t15 * t2;
        t17 = t16 - t9;
        t18 = t17 / 2;
        t19 = t18 + t14;
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* Path 1: More computations */
            t19 = t19 * 3 + 1;
            t19 = t19 / g_inv1;      /* Use invariant in conditional path */
        } else {
            /* Path 2: Different computations */
            t19 = t19 * 2 - 5;
            t19 = t19 % g_inv2;      /* Modulo - higher latency */
        }
        
        /* Continue dependency chain after conditional */
        int t20 = t19 * t5;
        int t21 = t20 + t12;
        int t22 = t21 - t16;
        int t23 = t22 / 4;
        int t24 = t23 + t8;
        int t25 = t24 * t3;
        int t26 = t25 - t11;
        int t27 = t26 / 6;
        int t28 = t27 + t15;
        int t29 = t28 * t6;
        int t30 = t29 - t18;
        
        /* Final accumulation with array access */
        sum += t30 + a[i] + b[i] + c[i];
    }
    
    return sum;
}

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold))
int higher_latency_op(int x, int y) {
    /* Complex operation that won't be inlined */
    return (x * x + y * y) / (x + y + 1);
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* restrict a, int* restrict b, int* restrict c, int n) {
    int sum = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Another set of temporaries */
        int u0, u1, u2, u3, u4, u5, u6, u7, u8, u9;
        
        /* Mix with higher latency function call */
        u0 = a[i] + b[i];
        u1 = higher_latency_op(u0, g_inv1);
        u2 = u1 * c[i];
        u3 = higher_latency_op(u2, g_inv2);
        u4 = u3 + a[i-1];
        u5 = u4 * b[i+1];
        u6 = higher_latency_op(u5, g_inv3);
        u7 = u6 - c[i];
        u8 = u7 / 11;                /* Division */
        u9 = u8 % 13;                /* Modulo */
        
        /* Conditional with both paths having computations */
        if (u9 > 100) {
            u9 = u9 * 2 + higher_latency_op(u9, i);
        } else {
            u9 = u9 / 2 - higher_latency_op(i, u9);
        }
        
        /* More operations */
        int u10 = u9 * u3;
        int u11 = u10 + u6;
        int u12 = u11 - u1;
        int u13 = higher_latency_op(u12, u4);
        int u14 = u13 * u8;
        
        sum += u14;
    }
    
    return sum;
}

/* Simple deterministic RNG for array initialization */
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

int main(void) {
    /* Allocate and initialize arrays */
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand() % 1000;
        b[i] = lcg_rand() % 1000;
        c[i] = lcg_rand() % 1000;
    }
    
    /* Call hot functions multiple times to ensure execution */
    int total_sum = 0;
    
    /* Multiple calls to ensure loop is compiled and executed */
    for (int iter = 0; iter < 10; iter++) {
        total_sum += compute_loop(a, b, c, SIZE);
        total_sum += compute_loop2(a, b, c, SIZE);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) % 1000;
            b[i] = (b[i] * 3) % 1000;
            c[i] = (c[i] + 7) % 1000;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_sum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
