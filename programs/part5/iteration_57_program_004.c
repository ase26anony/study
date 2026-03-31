/* test_modulo_sched.c
 * Designed to trigger modulo scheduling move edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fdump-rtl-sms-details test_modulo_sched.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Loop-invariant value that will be used across iterations */
static int GLOBAL_INVARIANT = 7;

/* High-latency operation to encourage move insertion */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division has higher latency than arithmetic */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n, int invariant) {
    int result = 0;
    
    /* Main loop with high register pressure and loop-carried dependencies */
    for (int i = 2; i < n - 2; i++) {
        /* Declare many temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Chain of dependent operations with loop-invariant value */
        t0 = a[i] + invariant;           /* Use invariant across iterations */
        t1 = t0 * b[i];
        t2 = t1 - c[i];
        t3 = t2 + a[i+1];
        t4 = t3 * b[i-1];
        t5 = t4 - c[i+1];
        
        /* High-latency operation mixing with arithmetic */
        t6 = high_latency_op(t5, invariant);
        
        /* Continue dependency chain */
        t7 = t6 + a[i-1];
        t8 = t7 * b[i+1];
        t9 = t8 - c[i-1];
        
        /* More temporaries to increase pressure */
        t10 = t9 + t0;
        t11 = t10 * t1;
        t12 = t11 - t2;
        t13 = t12 + t3;
        t14 = t13 * t4;
        t15 = t14 - t5;
        t16 = t15 + t6;
        t17 = t16 * t7;
        t18 = t17 - t8;
        t19 = t18 + t9;
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* Path 1: More computations */
            t10 = t10 + t15;
            t11 = t11 * t16;
            t12 = high_latency_op(t12, t17);
            result += t10 + t11 + t12;
        } else {
            /* Path 2: Different computations */
            t13 = t13 - t18;
            t14 = t14 * t19;
            t15 = high_latency_op(t15, invariant);
            result += t13 + t14 + t15;
        }
        
        /* Cross-iteration dependency through array */
        a[i] = t19 & 0xFF;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        a[i] = seed % 1000;
        b[i] = (seed >> 8) % 1000;
        c[i] = (seed >> 16) % 1000;
    }
}

int main() {
    const int SIZE = 1024;
    int a[SIZE], b[SIZE], c[SIZE];
    int total = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < 10; iter++) {
        /* Vary the invariant slightly each call */
        int invariant = GLOBAL_INVARIANT + (iter % 3);
        total += compute_loop(a, b, c, SIZE, invariant);
        
        /* Modify arrays slightly between calls */
        for (int i = 0; i < SIZE; i++) {
            a[i] = (a[i] + 1) & 0xFF;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
