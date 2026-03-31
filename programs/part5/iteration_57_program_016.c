/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global invariant value to force move creation */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries for register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Chain of dependent operations with loop-invariant */
        t0 = a[i] + GLOBAL_INVARIANT;           /* Use invariant */
        t1 = t0 * b[i];                         /* Dependent on t0 */
        t2 = t1 - c[i];                         /* Dependent on t1 */
        t3 = t2 + a[i-1];                       /* Array with offset */
        t4 = t3 * b[i+1];                       /* Array with offset */
        t5 = t4 / (c[i] + 2);                   /* Higher latency division */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More dependent operations */
            t6 = t5 + a[i] * 3;
            t7 = t6 - b[i-1];
            t8 = t7 * GLOBAL_INVARIANT;         /* Another invariant use */
            t9 = t8 / 4;                        /* Another division */
            t10 = t9 + t0;                      /* Cross-path dependency */
        } else {
            /* Path 2: Different chain */
            t6 = t5 * 2;
            t7 = t6 + c[i+1];
            t8 = t7 - GLOBAL_INVARIANT;         /* Invariant in else path */
            t9 = t8 * 5;
            t10 = t9 / 3;                       /* Division in else path */
        }
        
        /* Continue dependency chain after conditional */
        t11 = t10 + t2;
        t12 = t11 * t4;
        t13 = t12 - a[i];
        t14 = t13 + b[i];
        t15 = t14 * c[i];
        
        /* Mix operations with different modeled latencies */
        t16 = t15 % 17;                         /* Modulo operation */
        t17 = t16 + t8;
        t18 = t17 * t9;
        t19 = t18 - t11;
        
        /* Final accumulation with cross-iteration dependency */
        result += t19;
        
        /* Create anti-dependencies for next iteration */
        a[i] = t19 & 0xFF;  /* Modify array for next iteration use */
    }
    
    return result;
}

/* Non-inlineable function to model higher latency */
__attribute__((noinline, cold))
int helper_func(int x) {
    return (x * 3) / 2;
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int v0, v1, v2, v3, v4, v5, v6, v7, v8, v9;
        
        /* Long dependency chain */
        v0 = a[i] + GLOBAL_INVARIANT;
        v1 = helper_func(v0);                   /* Function call */
        v2 = v1 * b[i];
        v3 = v2 - c[i];
        v4 = v3 + GLOBAL_INVARIANT;
        v5 = v4 / (i + 1);                      /* Division with variable */
        v6 = v5 * v2;
        v7 = v6 % 13;                           /* Modulo */
        v8 = v7 + v3;
        v9 = v8 - v1;
        
        /* Conditional with both paths having computations */
        if (v9 > 0) {
            sum += v9 * 2;
        } else {
            sum -= v9 / 2;
        }
        
        /* Cross-iteration store/load */
        if (i > 0) {
            b[i] = a[i-1] + sum % 256;
        }
    }
    
    return sum;
}

/* Simple deterministic RNG for array initialization */
int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int seed = 42;
    int total = 0;
    
    /* Initialize arrays with deterministic pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call hot functions multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total += compute_loop(a, b, c, SIZE);
        total += compute_loop2(a, b, c, SIZE);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
