/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global invariant value to create cross-iteration dependencies */
int GLOBAL_INVARIANT = 7;

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i] + GLOBAL_INVARIANT;      /* Uses invariant */
        int t1 = t0 * b[i];                    /* Depends on t0 */
        int t2 = t1 - c[i];                    /* Depends on t1 */
        int t3 = t2 + a[i-1];                  /* Depends on t2, uses offset */
        int t4 = t3 * b[i+1];                  /* Depends on t3, uses offset */
        int t5 = t4 / (GLOBAL_INVARIANT + 1);  /* Depends on t4, uses invariant */
        int t6 = t5 - t2;                      /* Depends on t5 and earlier t2 */
        int t7 = t6 * 3;                       /* Depends on t6 */
        int t8 = t7 + a[i];                    /* Depends on t7 */
        int t9 = t8 - b[i-1];                  /* Depends on t8, uses offset */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations */
            int t10 = t9 * 2;
            int t11 = t10 + c[i+1];
            int t12 = t11 - GLOBAL_INVARIANT;
            result += t12;
        } else {
            /* Path 2: Different computations */
            int t10 = t9 / 2;
            int t11 = t10 * c[i];
            int t12 = t11 + GLOBAL_INVARIANT;
            result += t12;
        }
        
        /* Additional operations to increase register pressure */
        int t13 = t9 * t3;
        int t14 = t13 + t6;
        int t15 = t14 - t0;
        result ^= t15;  /* Use result to prevent elimination */
    }
    
    return result;
}

/* High-latency operation to encourage moves */
__attribute__((noinline, cold))
int slow_division(int a, int b) {
    /* Simulate higher latency operation */
    return a % (b + 1);
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int result = 0;
    int invariant_local = GLOBAL_INVARIANT * 2;
    
    for (int i = 0; i < n; i++) {
        /* Chain with mixed operations including high-latency */
        int t0 = a[i] + invariant_local;
        int t1 = b[i] * t0;
        int t2 = slow_division(t1, invariant_local);  /* High latency */
        int t3 = c[i] - t2;
        int t4 = t3 * GLOBAL_INVARIANT;
        int t5 = t4 + t0;
        int t6 = t5 - t2;
        int t7 = t6 * 3;
        int t8 = t7 / 2;
        
        /* Another conditional */
        if (t8 > 0) {
            int t9 = t8 + a[(i + 1) % n];
            int t10 = t9 * b[(i + 2) % n];
            result += t10;
        } else {
            int t9 = t8 - c[(i + 3) % n];
            int t10 = t9 / (invariant_local + 1);
            result += t10;
        }
        
        /* More temporaries */
        int t11 = t8 * t3;
        int t12 = t11 + t6;
        int t13 = t12 - t0;
        int t14 = t13 & 0xFF;
        result ^= t14;
    }
    
    return result;
}

/* Simple deterministic RNG for array initialization */
int simple_rand(int* seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7FFFFFFF;
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
        
        /* Modify arrays slightly to avoid complete redundancy */
        a[iter % SIZE] ^= iter;
        b[(iter + 1) % SIZE] += iter;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
