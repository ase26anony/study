/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

/* Global invariant value to force move creation */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0 = a[i];
        int t1 = t0 + GLOBAL_INVARIANT;  /* Use invariant in chain */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];
        int t5 = t4 * b[i+1];
        int t6 = t5 / (c[i] + 1);        /* Higher latency division */
        int t7 = t6 - GLOBAL_INVARIANT;  /* Another invariant use */
        int t8 = t7 * t3;
        int t9 = t8 + t1;
        int t10 = t9 - t4;
        int t11 = t10 * 3;
        int t12 = t11 + t6;
        int t13 = t12 - t8;
        int t14 = t13 * 2;
        int t15 = t14 + t11;
        
        /* Conditional creating multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 * a[i];
            int t17 = t16 + b[i-1];
            int t18 = t17 - c[i+1];
            result += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 / (GLOBAL_INVARIANT + 1); /* Another division */
            int t17 = t16 * 5;
            int t18 = t17 + t15;
            result += t18;
        }
        
        /* Cross-iteration dependency */
        a[i] = t15 % 256;  /* Modulo creates loop-carried dependency */
    }
    
    return result;
}

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold))
int slow_operation(int x, int y) {
    /* Complex enough to not be inlined */
    for (int i = 0; i < 3; i++) {
        x = (x * y) % 31;
    }
    return x;
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int result = 0;
    int inv1 = GLOBAL_INVARIANT * 2;
    int inv2 = GLOBAL_INVARIANT + 3;
    
    for (int i = 2; i < n - 2; i++) {
        /* Different dependency chain */
        int t0 = slow_operation(a[i], inv1);  /* Function call */
        int t1 = t0 + b[i];
        int t2 = t1 * c[i];
        int t3 = t2 - a[i-1];
        int t4 = t3 + b[i+1];
        int t5 = t4 * inv2;  /* Another invariant */
        int t6 = t5 / (c[i-1] + 1);
        int t7 = t6 + t2;
        int t8 = t7 - t4;
        int t9 = t8 * 7;
        
        /* Nested conditionals */
        if (t9 > 0) {
            if (a[i] & 1) {
                result += t9 * 2;
            } else {
                result += t9 / 2;
            }
        } else {
            result -= t9;
        }
        
        /* Multiple array updates with dependencies */
        b[i] = (t9 + a[i]) % 127;
        c[i] = (t9 - b[i-1]) & 255;
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
int simple_rand(int seed) {
    return (seed * 1103515245 + 12345) & 0x7fffffff;
}

int main() {
    /* Allocate and initialize arrays */
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = simple_rand(seed);
        a[i] = seed % 1000;
        seed = simple_rand(seed);
        b[i] = seed % 1000;
        seed = simple_rand(seed);
        c[i] = seed % 1000;
    }
    
    /* Call hot functions multiple times to ensure execution */
    int total_result = 0;
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = (GLOBAL_INVARIANT * 13 + 17) % 47;
        
        total_result += compute_loop(a, b, c, SIZE);
        total_result += compute_loop2(a, b, c, SIZE);
        
        /* Shuffle arrays slightly to change patterns */
        for (int i = 1; i < SIZE - 1; i++) {
            a[i] = (a[i] + b[i-1]) % 1000;
            b[i] = (b[i] + c[i+1]) % 1000;
        }
    }
    
    printf("Result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
