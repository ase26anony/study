/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create loop-invariant values */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) int slow_operation(int x, int y) {
    /* Division operation with higher latency */
    return (x % y) + (x / y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline)) int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Declare many temporaries to increase register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Start dependency chain with loop-invariant values */
        t0 = a[i] + g_invariant1;           /* Use invariant 1 */
        t1 = t0 * b[i];                     /* Dependent on t0 */
        t2 = t1 - g_invariant2;             /* Use invariant 2 */
        t3 = t2 + c[i];                     /* Dependent on t2 */
        t4 = t3 * a[i-1];                   /* Array access with offset */
        t5 = t4 / (b[i+1] + 1);             /* Array access with offset */
        
        /* Higher latency operation mixing invariant */
        t6 = slow_operation(t5, g_invariant3); /* High latency + invariant 3 */
        
        /* Continue dependency chain */
        t7 = t6 + a[i+1];                   /* Another offset access */
        t8 = t7 * t3;                       /* Cross-dependency */
        t9 = t8 - t1;                       /* More dependencies */
        
        /* Additional temporaries to increase pressure */
        t10 = t9 + b[i-1];
        t11 = t10 * c[i];
        t12 = t11 - a[i];
        t13 = t12 + t4;
        t14 = t13 * t7;
        t15 = t14 / (t8 + 1);
        t16 = t15 - t2;
        t17 = t16 + t5;
        t18 = t17 * t10;
        t19 = t18 - t13;
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* Path 1: More computations */
            t19 = t19 * 3 + g_invariant1;
            t19 = slow_operation(t19, 5);   /* Another high latency op */
        } else {
            /* Path 2: Different computations */
            t19 = t19 / 2 - g_invariant2;
            t19 = t19 * t19 + 1;
        }
        
        /* Final accumulation with cross-iteration dependency */
        result += t19 + (i % 8);            /* Modulo creates dependency */
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
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
    const int SIZE = 1024;
    int a[SIZE], b[SIZE], c[SIZE];
    int total_result = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 1) & 0xFF;
        g_invariant3 = (g_invariant3 * 7 + 1) & 0xFF;
        
        /* Call the hot loop function */
        total_result += compute_hot_loop(a, b, c, SIZE);
        
        /* Rotate arrays to change data dependencies */
        int temp = a[0];
        for (int i = 0; i < SIZE - 1; i++) {
            a[i] = a[i + 1];
        }
        a[SIZE - 1] = temp;
    }
    
    printf("Final result: %d\n", total_result);
    return 0;
}
