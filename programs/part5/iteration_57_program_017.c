/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

/* Global invariant value to force move creation */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0 = a[i] + GLOBAL_INVARIANT;  /* Loop-invariant use */
        int t1 = t0 * b[i];
        int t2 = t1 - c[i];
        int t3 = t2 + a[i-1];              /* Array access with offset */
        int t4 = t3 * b[i+1];              /* Another offset access */
        int t5 = t4 / (GLOBAL_INVARIANT + 1); /* Higher latency division */
        int t6 = t5 + t2;
        int t7 = t6 * 3;
        int t8 = t7 - t4;
        int t9 = t8 + t1;
        int t10 = t9 * 2;
        int t11 = t10 - t3;
        int t12 = t11 + t6;
        int t13 = t12 * t5;
        int t14 = t13 - t8;
        int t15 = t14 + t10;
        
        /* Conditional creating multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 * a[i];
            int t17 = t16 + b[i-1];
            int t18 = t17 % (GLOBAL_INVARIANT + 2); /* Another high latency op */
            result += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 + c[i+1];
            int t17 = t16 * 2;
            int t18 = t17 - a[i];
            int t19 = t18 % (GLOBAL_INVARIANT + 3);
            result += t19 + t15;
        }
        
        /* Cross-iteration dependency */
        a[i] = t15 & 0xFF;  /* Store back to create loop-carried dependency */
    }
    
    return result;
}

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold))
int high_latency_op(int x, int y) {
    return (x * y) / (GLOBAL_INVARIANT + 5);
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int result = 0;
    int inv = GLOBAL_INVARIANT * 2;  /* Another invariant */
    
    for (int i = 2; i < n - 2; i++) {
        /* Long dependency chain with mixed operations */
        int t0 = a[i] + inv;
        int t1 = high_latency_op(t0, b[i]);  /* Function call adds latency */
        int t2 = t1 - c[i];
        int t3 = t2 * a[i-2];
        int t4 = t3 + b[i+2];
        int t5 = t4 % (inv + 1);  /* Modulo operation */
        int t6 = t5 * t2;
        int t7 = t6 + t3;
        int t8 = t7 - t4;
        int t9 = t8 * 2;
        int t10 = t9 + t1;
        int t11 = t10 % 13;  /* Another modulo */
        int t12 = t11 * t5;
        
        /* Complex conditional with computations in both paths */
        if (t12 > 0) {
            int t13 = t12 + a[i-1];
            int t14 = t13 * b[i];
            result += t14;
        } else {
            int t13 = t12 - c[i+1];
            int t14 = high_latency_op(t13, inv);
            result += t14;
        }
        
        /* Multiple stores creating dependencies */
        b[i] = t12 & 0xFF;
        c[i] = (t12 >> 8) & 0xFF;
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
unsigned int simple_rand(unsigned int* seed) {
    *seed = *seed * 1103515245 + 12345;
    return *seed;
}

int main() {
    /* Allocate and initialize arrays */
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    
    unsigned int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand(&seed) % 1000;
        b[i] = simple_rand(&seed) % 1000;
        c[i] = simple_rand(&seed) % 1000;
    }
    
    /* Call hot functions multiple times to ensure execution */
    int total_result = 0;
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = (GLOBAL_INVARIANT * 13 + 17) % 100;
        
        total_result += compute_loop(a, b, c, SIZE);
        total_result += compute_loop2(a, b, c, SIZE);
        
        /* Slightly modify arrays to prevent complete optimization */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) % 1000;
        }
    }
    
    printf("Result: %d\n", total_result);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
