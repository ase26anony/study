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
        int t1 = t0 + GLOBAL_INVARIANT;  /* Use invariant value */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];
        int t5 = t4 * b[i+1];
        int t6 = t5 - c[i-1];
        int t7 = t6 + GLOBAL_INVARIANT;  /* Another invariant use */
        int t8 = t7 * 3;
        int t9 = t8 / 2;                 /* Higher latency operation */
        
        /* Conditional creating multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations */
            int t10 = t9 + a[i+1];
            int t11 = t10 * GLOBAL_INVARIANT;
            int t12 = t11 % 5;           /* Another higher latency op */
            result += t12;
        } else {
            /* Path 2: Different computations */
            int t10 = t9 - b[i];
            int t11 = t10 * 2;
            int t12 = t11 + GLOBAL_INVARIANT;
            result += t12;
        }
        
        /* Additional dependent chain to increase critical path */
        int t13 = t9 * 7;
        int t14 = t13 + result;
        int t15 = t14 % 11;              /* Modulo - higher latency */
        result = t15 ^ (result & 0xFF);
    }
    
    return result;
}

/* Helper to initialize arrays deterministically */
void init_arrays(int* a, int* b, int* c, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int total = 0;
    
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure execution */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = (GLOBAL_INVARIANT * 13 + 17) & 0xFF;
        
        total += compute_loop(a, b, c, SIZE);
        
        /* Rotate arrays to change data dependencies */
        int temp = a[0];
        for (int i = 0; i < SIZE - 1; i++) {
            a[i] = a[i + 1];
        }
        a[SIZE - 1] = temp;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
