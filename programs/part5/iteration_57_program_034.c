/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Loop-invariant value that will be used across iterations */
static int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
        
        /* Start chain with loop-invariant value - may create move edges */
        t0 = GLOBAL_INVARIANT;
        
        /* Chain of dependent operations with array accesses */
        t1 = t0 + a[i];          /* Use invariant in chain */
        t2 = t1 * b[i];          /* Multiplication has higher latency */
        t3 = t2 - c[i];          /* Continue dependency chain */
        t4 = t3 + a[i+1];        /* Access with offset */
        t5 = t4 * b[i-1];        /* Another offset access */
        t6 = t5 / GLOBAL_INVARIANT; /* Division with invariant - higher latency */
        t7 = t6 + t0;            /* Reuse t0 (invariant) */
        t8 = t7 * 3;             /* Constant multiplication */
        t9 = t8 - t2;            /* Cross-chain dependency */
        t10 = t9 + a[i] * 2;     /* More computation */
        t11 = t10 % 13;          /* Modulo operation - different latency */
        t12 = t11 | t5;          /* Bitwise operation */
        t13 = t12 & 0xFF;        /* Mask operation */
        t14 = t13 ^ t8;          /* XOR operation */
        t15 = t14 << 2;          /* Shift operation */
        
        /* Create conditional basic block inside loop */
        if (t5 & 1) {  /* Condition based on computed value */
            /* True path with more computations */
            t15 = t15 + (t3 * 2);
            result += t15 * 2;
        } else {
            /* False path with different computations */
            t15 = t15 - (t6 / 2);
            result += t15 / 3;
        }
        
        /* Additional computation after conditional */
        result += t15;
        
        /* More operations to increase critical path */
        int extra1 = t15 * a[i];
        int extra2 = extra1 + b[i];
        int extra3 = extra2 - c[i];
        result += extra3;
    }
    
    return result;
}

/* Simple deterministic PRNG for array initialization */
static unsigned int seed = 12345;
static unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

int main() {
    /* Initialize arrays with deterministic pseudo-random values */
    int a[SIZE], b[SIZE], c[SIZE];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = simple_rand() % 100;
        b[i] = simple_rand() % 100;
        c[i] = simple_rand() % 100;
    }
    
    /* Call hot function multiple times to ensure it's compiled as hot */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify arrays slightly each iteration */
        a[iter % SIZE] = iter;
        b[iter % SIZE] = iter * 2;
        
        total_result += compute_loop(a, b, c, SIZE);
        
        /* Prevent compiler from optimizing away the loop */
        if (total_result % 1000000 == 0) {
            printf("Intermediate: %d\n", total_result);
        }
    }
    
    printf("Final result: %d\n", total_result);
    return 0;
}
