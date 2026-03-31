/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

/* Global invariant value to force move creation */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_hot(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with loop-carried dependencies */
        int t0 = a[i] + GLOBAL_INVARIANT;  /* Use invariant value */
        int t1 = t0 * b[i];
        int t2 = t1 - c[i];
        int t3 = t2 + a[i-1];              /* Array access with offset */
        int t4 = t3 * b[i+1];              /* Another offset access */
        int t5 = t4 / (GLOBAL_INVARIANT + 1); /* Higher latency division */
        int t6 = t5 + t0;                  /* Cross-iteration dependency hint */
        int t7 = t6 * t2;
        int t8 = t7 - t4;
        int t9 = t8 + t1;
        int t10 = t9 * t3;
        int t11 = t10 / (t5 + 2);          /* Another division */
        int t12 = t11 + t6;
        int t13 = t12 * t8;
        int t14 = t13 - t10;
        int t15 = t14 + t9;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            t15 = t15 * 3 + GLOBAL_INVARIANT;
            t15 = t15 / 2;                 /* Higher latency op */
        } else {
            /* Path 2: Different computations */
            t15 = t15 + GLOBAL_INVARIANT * 2;
            t15 = t15 - t14 / 4;           /* Another division */
        }
        
        /* Final accumulation with array dependency */
        result += t15 + a[i] + b[i] + c[i];
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
int simple_rand(int seed) {
    return (seed * 1103515245 + 12345) & 0x7FFFFFFF;
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
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = simple_rand(seed);
        a[i] = seed % 100;
        seed = simple_rand(seed);
        b[i] = seed % 100;
        seed = simple_rand(seed);
        c[i] = seed % 100;
    }
    
    /* Call hot function multiple times to ensure execution */
    int total = 0;
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify GLOBAL_INVARIANT slightly each iteration */
        GLOBAL_INVARIANT = (GLOBAL_INVARIANT * 3 + 1) % 13;
        
        /* Call the hot computation function */
        total += compute_hot(a, b, c, SIZE);
        
        /* Slightly modify arrays to prevent complete optimization */
        a[iter % SIZE] = iter;
    }
    
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
