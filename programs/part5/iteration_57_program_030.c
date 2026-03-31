/* test_modulo_sched.c - Program to trigger modulo scheduling move edge logging */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to create loop-invariant value */
int GLOBAL_INVARIANT = 7;

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant value from global */
    int invariant = GLOBAL_INVARIANT;
    
    /* Large iteration count to encourage modulo scheduling */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + invariant;          /* Use invariant - may create move */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];
        int t5 = t4 * b[i+1];
        int t6 = t5 - c[i-1];
        int t7 = t6 + invariant;          /* Another use of invariant */
        
        /* Operation with potentially higher latency */
        int t8 = t7 % 13;                 /* Modulo operation */
        
        /* More dependent operations */
        int t9 = t8 * t3;
        int t10 = t9 + t1;
        int t11 = t10 - t4;
        int t12 = t11 * t6;
        int t13 = t12 + t7;
        int t14 = t13 - t8;
        
        /* Conditional to create multiple basic blocks */
        if (t14 & 1) {
            /* Path 1: More computations */
            int t15 = t14 * 3;
            int t16 = t15 + invariant;    /* Another invariant use */
            int t17 = t16 / 5;            /* Division - higher latency */
            result += t17;
        } else {
            /* Path 2: Different computations */
            int t15 = t14 + 7;
            int t16 = t15 * invariant;    /* Another invariant use */
            int t17 = t16 - 11;
            result += t17;
        }
        
        /* Cross-iteration dependency */
        a[i] = t14 + result % 17;         /* Store with dependency */
    }
    
    return result;
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int result = 0;
    int invariant = GLOBAL_INVARIANT + 3;
    
    for (int i = 2; i < n - 2; i++) {
        /* Different dependency chain */
        int t0 = b[i];
        int t1 = a[i-2] + invariant;
        int t2 = t0 * t1;
        int t3 = c[i+1] - t2;
        int t4 = t3 + invariant * 2;
        int t5 = t4 % 19;
        int t6 = t5 + a[i+1];
        int t7 = t6 * b[i-1];
        int t8 = t7 - c[i];
        
        /* Nested conditionals for more complex CFG */
        if (t8 > 0) {
            int t9 = t8 + invariant;
            if (t9 & 2) {
                result += t9 * 3;
            } else {
                result += t9 / 4;
            }
        } else {
            int t9 = t8 - invariant;
            result += t9 * 5;
        }
        
        /* Another invariant use in store */
        b[i] = t8 + invariant;
    }
    
    return result;
}

int main() {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = lcg_rand() % 100;
        b[i] = lcg_rand() % 100;
        c[i] = lcg_rand() % 100;
    }
    
    int total = 0;
    
    /* Call hot functions multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        total += compute_loop(a, b, c, SIZE);
        total += compute_loop2(a, b, c, SIZE);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) % 100;
        }
    }
    
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
