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
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with chain dependencies */
        int t0 = a[i];
        int t1 = t0 + GLOBAL_INVARIANT;  /* Use invariant value */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];           /* Array access with offset */
        int t5 = t4 * b[i+1];           /* Another offset access */
        int t6 = t5 / 3;                /* Higher latency division */
        int t7 = t6 - GLOBAL_INVARIANT; /* Another invariant use */
        int t8 = t7 * 2;
        int t9 = t8 + t0;               /* Loop-carried dependency pattern */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations */
            int t10 = t9 * 3;
            int t11 = t10 - b[i];
            int t12 = t11 % 17;         /* Another high latency op */
            result += t12;
        } else {
            /* Path 2: Different computations */
            int t10 = t9 / 5;
            int t11 = t10 + c[i-1];
            int t12 = t11 * 2;
            result += t12;
        }
        
        /* Additional operations to increase pressure */
        int t13 = result * 2;
        int t14 = t13 + a[i];
        int t15 = t14 - t9;
        result = t15 & 0xFFF;          /* Prevent overflow */
    }
    
    return result;
}

/* Helper to initialize arrays */
void init_arrays(int* a, int* b, int* c, int n) {
    /* Simple deterministic PRNG */
    unsigned seed = 12345;
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
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times */
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = (iter % 13) + 5;
        
        /* Call the computation loop */
        int res = compute_loop(a, b, c, SIZE);
        total += res;
        
        /* Slightly modify arrays to prevent complete optimization */
        a[iter % SIZE] = iter;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
