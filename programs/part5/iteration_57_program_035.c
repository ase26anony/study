/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global invariant value to create cross-iteration dependencies */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with chain dependencies */
        int t0 = a[i] + GLOBAL_INVARIANT;      /* Use invariant value */
        int t1 = t0 * b[i];                    /* Chain dependency */
        int t2 = t1 - c[i];                    /* Another dependency */
        int t3 = t2 + a[i-1];                  /* Use previous element */
        int t4 = t3 * b[i+1];                  /* Use next element */
        int t5 = t4 / (GLOBAL_INVARIANT + 1);  /* Higher latency division */
        int t6 = t5 + t2;                      /* Cross-temporary dependency */
        int t7 = t6 * 3;                       /* More computation */
        int t8 = t7 - t1;                      /* Another cross-dependency */
        int t9 = t8 + a[i] * 2;                /* Memory access */
        int t10 = t9 / (t5 & 0xF + 1);         /* Variable divisor */
        
        /* Conditional to create multiple basic blocks */
        if (t10 & 1) {
            /* Path 1: More computations */
            int t11 = t10 * GLOBAL_INVARIANT;  /* Use invariant again */
            int t12 = t11 + b[i-1];            /* Another memory access */
            int t13 = t12 - t8;                /* Cross-iteration potential */
            result += t13;
        } else {
            /* Path 2: Different computations */
            int t11 = t10 / (GLOBAL_INVARIANT | 1); /* Division */
            int t12 = t11 * c[i+1];            /* Memory access */
            int t13 = t12 + t6;                /* Different dependency */
            result += t13 * 2;
        }
        
        /* Additional computations outside condition */
        int t14 = t10 + result;
        int t15 = t14 * (GLOBAL_INVARIANT % 5 + 1);
        result = t15 & 0xFFF;  /* Keep result bounded */
    }
    
    return result;
}

/* Non-inlineable cold function to create higher latency operations */
__attribute__((noinline, cold))
int cold_helper(int x, int y) {
    /* Complex enough to not be inlined */
    return (x * y) / (x - y + 1);
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_hot_loop2(int* a, int* b, int* c, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Different dependency chain */
        int v0 = a[i] ^ GLOBAL_INVARIANT;
        int v1 = v0 + cold_helper(b[i], GLOBAL_INVARIANT); /* Call adds latency */
        int v2 = v1 * (c[i] + 1);
        int v3 = v2 - (i * GLOBAL_INVARIANT); /* Loop variant with invariant */
        int v4 = v3 / (v1 & 0x7F + 1);
        
        /* Nested condition */
        if (v4 > 0) {
            int v5 = v4 + cold_helper(v3, v2);
            sum += v5;
        } else {
            int v5 = v4 * 2 - GLOBAL_INVARIANT;
            sum += v5;
        }
        
        /* More computations */
        int v6 = sum + v4;
        int v7 = v6 * (GLOBAL_INVARIANT % 3 + 2);
        sum = v7 & 0xFFFF;
    }
    
    return sum;
}

/* Simple deterministic RNG for array initialization */
int simple_rand(int seed) {
    return (seed * 1103515245 + 12345) & 0x7FFFFFFF;
}

int main() {
    /* Allocate and initialize arrays */
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    
    /* Initialize with deterministic pseudo-random values */
    int seed = 42;
    for (int i = 0; i < SIZE; i++) {
        seed = simple_rand(seed);
        a[i] = (seed % 1000) - 500;
        seed = simple_rand(seed);
        b[i] = (seed % 1000) - 500;
        seed = simple_rand(seed);
        c[i] = (seed % 1000) - 500;
    }
    
    /* Call hot functions multiple times to ensure execution */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = (GLOBAL_INVARIANT * 13 + 17) & 0xFF;
        
        /* Call both hot functions */
        total_result += compute_hot_loop(a, b, c, SIZE);
        total_result += compute_hot_loop2(a, b, c, SIZE);
        
        /* Modify arrays slightly to avoid complete optimization */
        a[iter % SIZE] = total_result & 0xFF;
        b[iter % SIZE] = (total_result >> 8) & 0xFF;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
