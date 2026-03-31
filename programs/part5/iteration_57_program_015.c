/* test_modulo_sched.c - Target program for modulo scheduler coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create loop-invariant values */
int g_invariant1 = 12345;
int g_invariant2 = 67890;
int g_invariant3 = 54321;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division has higher latency than basic arithmetic */
    return (x % (y | 1)) + (y % (x | 1));
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int target_loop(int* a, int* b, int* c, int* d, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 2; i < n - 2; i++) {
        /* Declare many temporaries to increase register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Start chain with loop-invariant values - may create moves */
        t0 = g_invariant1 + a[i];
        t1 = t0 * g_invariant2;
        t2 = t1 - g_invariant3;
        
        /* Array accesses with different offsets */
        t3 = t2 + b[i-1];
        t4 = t3 * b[i];
        t5 = t4 + b[i+1];
        
        /* High latency operation in the chain */
        t6 = high_latency_op(t5, c[i]);
        
        /* More arithmetic creating long dependency chain */
        t7 = t6 * 7;
        t8 = t7 - a[i-2];
        t9 = t8 + a[i+2];
        t10 = t9 * 3;
        t11 = t10 / 5;  /* Division has higher latency */
        t12 = t11 + d[i];
        
        /* Conditional to create multiple basic blocks */
        if (t12 & 1) {
            /* Path 1: More computations */
            t13 = t12 * 11;
            t14 = t13 + c[i-1];
            t15 = high_latency_op(t14, g_invariant1);
            t16 = t15 - b[i];
            t17 = t16 * 2;
            result += t17;
        } else {
            /* Path 2: Different computations but still dependent */
            t13 = t12 / 3;  /* Another division */
            t14 = t13 * g_invariant2;
            t15 = t14 + a[i];
            t16 = high_latency_op(t15, g_invariant3);
            t17 = t16 - 1;
            result += t17;
        }
        
        /* Continue chain after conditional */
        t18 = t17 * 5;
        t19 = t18 + result;
        
        /* Use t19 to prevent dead code elimination */
        result = t19 & 0xFFF;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int* d, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0x7FFF;
        c[i] = seed & 0x7FFF;
        d[i] = (a[i] + b[i] + c[i]) & 0xFFFF;
    }
}

int main() {
    const int SIZE = 1024;
    int a[SIZE], b[SIZE], c[SIZE], d[SIZE];
    int total_result = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, d, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = 12345 + iter;
        g_invariant2 = 67890 - iter;
        g_invariant3 = 54321 + iter * 2;
        
        total_result += target_loop(a, b, c, d, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    
    /* Also test with different array sizes */
    const int SIZE2 = 512;
    int a2[SIZE2], b2[SIZE2], c2[SIZE2], d2[SIZE2];
    init_arrays(a2, b2, c2, d2, SIZE2);
    total_result += target_loop(a2, b2, c2, d2, SIZE2);
    
    printf("Final result: %d\n", total_result);
    
    return 0;
}
