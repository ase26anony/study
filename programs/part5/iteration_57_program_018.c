/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop invariants */
int global_invariant1 = 7;
int global_invariant2 = 13;
int global_invariant3 = 19;

/* Hot function with complex loop body */
__attribute__((hot, noinline))
int compute_hot_loop(int* arr1, int* arr2, int* arr3, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0 = arr1[i];
        int t1 = t0 + global_invariant1;  /* Loop invariant use */
        int t2 = t1 * arr2[i];
        int t3 = t2 - global_invariant2;  /* Another invariant */
        int t4 = t3 / (arr3[i] + 1);      /* Higher latency division */
        int t5 = t4 ^ global_invariant3;  /* Another invariant */
        int t6 = t5 + arr1[i-1];          /* Array access with offset */
        int t7 = t6 * arr2[i+1];          /* Different offset */
        int t8 = t7 - t0;
        int t9 = t8 | t3;
        int t10 = t9 & t5;
        int t11 = t10 << 2;
        int t12 = t11 >> 1;
        int t13 = t12 + arr3[i];
        int t14 = t13 * t4;
        int t15 = t14 - t7;
        int t16 = t15 + t11;
        int t17 = t16 ^ t8;
        int t18 = t17 * 3;
        int t19 = t18 / (t2 + 1);         /* Another division */
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* Path 1: More computations */
            int t20 = t19 + global_invariant1;
            int t21 = t20 * t5;
            int t22 = t21 - arr1[i];
            result += t22 % 17;           /* Modulo operation */
        } else {
            /* Path 2: Different computations */
            int t23 = t19 - global_invariant2;
            int t24 = t23 ^ t8;
            int t25 = t24 * t3;
            result += t25 % 23;           /* Modulo operation */
        }
        
        /* Cross-iteration dependency */
        arr1[i] = t19 + result;
    }
    
    return result;
}

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold))
int higher_latency_op(int a, int b) {
    /* Complex operation that won't be inlined */
    return (a * b) + (a / (b + 1)) - (a % (b | 1));
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_hot_loop2(int* arr1, int* arr2, int* arr3, int n) {
    int result = 0;
    int invariant = global_invariant1 * global_invariant2;
    
    for (int i = 2; i < n - 2; i++) {
        /* Long dependency chain with mixed operations */
        int v0 = arr1[i];
        int v1 = v0 + invariant;
        int v2 = higher_latency_op(v1, arr2[i]);  /* Function call */
        int v3 = v2 * arr3[i];
        int v4 = v3 - global_invariant3;
        int v5 = v4 / (arr1[i-1] + 1);
        int v6 = v5 ^ v0;
        int v7 = v6 + arr2[i+1];
        int v8 = v7 * arr3[i-1];
        int v9 = v8 - v3;
        int v10 = v9 | v5;
        int v11 = v10 & v6;
        int v12 = v11 << 3;
        int v13 = v12 >> 2;
        int v14 = v13 + v7;
        int v15 = v14 * v4;
        
        /* Conditional with both paths having computations */
        if (v15 > 0) {
            int v16 = v15 + higher_latency_op(v8, v2);
            result += v16 % 29;
        } else {
            int v17 = v15 - higher_latency_op(v9, v3);
            result += v17 % 31;
        }
        
        /* Store creating loop-carried dependency */
        arr2[i] = v15 + result;
    }
    
    return result;
}

/* Simple PRNG for deterministic values */
static unsigned int seed = 12345;
static unsigned int prng() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

int main() {
    /* Allocate and initialize arrays */
    int* arr1 = malloc(SIZE * sizeof(int));
    int* arr2 = malloc(SIZE * sizeof(int));
    int* arr3 = malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = prng() % 1000;
        arr2[i] = prng() % 1000;
        arr3[i] = prng() % 1000;
    }
    
    int total_result = 0;
    
    /* Call hot functions multiple times to ensure execution */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        total_result += compute_hot_loop(arr1, arr2, arr3, SIZE);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < SIZE; i++) {
            arr1[i] = (arr1[i] + 1) % 1000;
        }
        
        total_result += compute_hot_loop2(arr1, arr2, arr3, SIZE);
        
        /* More modifications */
        for (int i = 0; i < SIZE; i++) {
            arr2[i] = (arr2[i] + 2) % 1000;
        }
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
