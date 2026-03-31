/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop-invariant values */
int g_invariant1 = 17;
int g_invariant2 = 23;
int g_invariant3 = 42;

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Complex operation that won't be inlined */
    return (x * y) ^ (x + y) ^ (x - y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int target_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Chain of dependent operations with loop-invariant values */
        t0 = a[i] + g_invariant1;           /* Use invariant */
        t1 = t0 * b[i];
        t2 = t1 - g_invariant2;             /* Use another invariant */
        t3 = t2 + c[i];
        t4 = t3 * a[i-1];                   /* Array access with offset */
        t5 = t4 / (b[i+1] + 1);             /* Division for higher latency */
        t6 = t5 ^ g_invariant3;             /* Use third invariant */
        t7 = t6 + a[i+1];                   /* Forward array access */
        
        /* High latency operation call */
        t8 = high_latency_op(t7, t3);
        
        /* More dependent operations */
        t9 = t8 * t5;
        t10 = t9 - t2;
        t11 = t10 + c[i-1];                 /* Backward array access */
        t12 = t11 * t4;
        t13 = t12 / (t6 + 2);               /* Another division */
        t14 = t13 ^ t8;
        t15 = t14 + b[i-1];
        t16 = t15 * t11;
        t17 = t16 - t9;
        t18 = t17 + a[i] * b[i];
        t19 = t18 / (c[i] + 3);
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* Path 1: More computations */
            t19 = t19 * 3 + g_invariant1;
            t19 = high_latency_op(t19, t10);
            result += t19 % 7;              /* Modulo for latency */
        } else {
            /* Path 2: Different computations */
            t19 = t19 / 2 - g_invariant2;
            t19 = t19 ^ t15;
            result += t19 & 0xF;
        }
        
        /* Additional computation to extend live ranges */
        result += (t5 + t10 + t15) & 0xFF;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int total_result = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 13 + 7) & 0xFF;
        g_invariant2 = (g_invariant2 * 17 + 11) & 0xFF;
        g_invariant3 = (g_invariant3 * 19 + 13) & 0xFF;
        
        total_result += target_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
