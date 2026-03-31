/* test_modulo_sched.c - Target program for modulo scheduler coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global invariant value to force move creation */
int GLOBAL_INVARIANT = 42;

/* Simple deterministic pseudo-random generator */
static inline int simple_rand(int seed) {
    return (seed * 1103515245 + 12345) & 0x7fffffff;
}

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int hot_loop_function(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant value from global - will create move edges */
    int invariant = GLOBAL_INVARIANT;
    
    /* High iteration count for overlapping */
    for (int i = 0; i < n; ++i) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + invariant;      /* Uses invariant across iterations */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 >> 2;
        int t5 = t4 | (i & 0xFF);
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations with dependencies */
            int t6 = t5 * 3;
            int t7 = t6 + a[i + 1];    /* Array access with offset */
            int t8 = t7 % 13;          /* Higher latency operation */
            int t9 = t8 ^ b[i - 1];    /* Another offset access */
            result += t9;
        } else {
            /* Path 2: Different computations but still dependent */
            int t6 = t5 / 7;           /* Higher latency division */
            int t7 = t6 - c[i + 1];
            int t8 = t7 << 1;
            int t9 = t8 & 0xFFFF;
            result -= t9;
        }
        
        /* Cross-iteration dependency chain */
        int t10 = result * 2;
        int t11 = t10 + (i % 8);
        int t12 = t11 ^ invariant;     /* Another use of invariant */
        int t13 = t12 | t5;
        
        /* Memory operations with different addressing modes */
        if (i > 0 && i < n - 1) {
            a[i] = t13 + b[i] - c[i];
        }
        
        /* More temporaries to increase register pressure */
        int t14 = t13 * 3;
        int t15 = t14 + 1;
        int t16 = t15 << 2;
        int t17 = t16 >> 1;
        int t18 = t17 & 0xFF;
        int t19 = t18 | 0x80;
        
        /* Final accumulation with invariant */
        result += t19 + (invariant % 17);
    }
    
    return result;
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int hot_loop_function2(int* a, int* b, int* c, int n) {
    int sum = 0;
    int inv = GLOBAL_INVARIANT * 2;
    
    for (int i = 2; i < n - 2; i += 1) {
        /* Long dependency chain with invariant */
        int v0 = a[i - 2];
        int v1 = v0 + inv;
        int v2 = v1 * b[i - 1];
        int v3 = v2 - c[i];
        int v4 = v3 * a[i + 1];
        int v5 = v4 + b[i + 2];
        int v6 = v5 % 19;              /* Higher latency */
        int v7 = v6 ^ inv;
        int v8 = v7 << 3;
        int v9 = v8 >> 1;
        int v10 = v9 | 0x7F;
        
        /* Conditional with computations in both paths */
        if (v10 > 1000) {
            int v11 = v10 * 5;
            int v12 = v11 / 3;         /* Higher latency */
            sum += v12;
        } else {
            int v11 = v10 + 100;
            int v12 = v11 * 2;
            sum -= v12;
        }
        
        /* More operations to increase pressure */
        int v13 = sum * 7;
        int v14 = v13 + i;
        int v15 = v14 ^ (inv >> 2);
        int v16 = v15 & 0xFFF;
        
        /* Store with dependency */
        if (i % 4 == 0) {
            c[i] = v16;
        }
    }
    
    return sum;
}

int main() {
    const int SIZE = 1024;
    
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with deterministic pseudo-random values */
    int seed = 123456;
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
    
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = 42 + (iter % 5);
        
        /* Call first hot function */
        total_result += hot_loop_function(a, b, c, SIZE);
        
        /* Call second hot function */
        total_result += hot_loop_function2(a, b, c, SIZE);
        
        /* Modify arrays slightly to prevent complete optimization */
        for (int i = 0; i < SIZE; i += 8) {
            a[i] += iter;
            b[i] -= iter;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
