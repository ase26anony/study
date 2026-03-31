/* test_modulo_sched.c
 * Designed to trigger move edge logging in GCC's modulo scheduler
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fdump-rtl-sms-details test_modulo_sched.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global invariant value to force move creation */
static int GLOBAL_INVARIANT = 17;

/* Non-inlineable function with higher latency */
static int __attribute__((noinline, cold)) high_latency_op(int x, int y) {
    /* Complex operation that won't be inlined */
    return (x * y) / (y + 1);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline)) 
static int target_loop(int* restrict a, int* restrict b, int* restrict c, int n) {
    int result = 0;
    int invariant = GLOBAL_INVARIANT;
    
    /* Main loop with high register pressure and dependencies */
    for (int i = 2; i < n - 2; i++) {
        /* Declare many temporaries to create register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Long chain of dependent operations */
        t0 = a[i] + invariant;          /* Use invariant value */
        t1 = t0 * b[i];
        t2 = t1 - c[i];
        t3 = t2 + a[i+1];
        t4 = t3 * b[i-1];
        t5 = t4 - c[i+1];
        
        /* High latency operation that may create move edges */
        t6 = high_latency_op(t5, invariant);
        
        /* Continue dependency chain */
        t7 = t6 + a[i-1];
        t8 = t7 * b[i+1];
        t9 = t8 - c[i-1];
        t10 = t9 + a[i+2];
        t11 = t10 * b[i-2];
        t12 = t11 - c[i+2];
        t13 = t12 + a[i-2];
        t14 = t13 * b[i];
        t15 = t14 - c[i];
        t16 = t15 + a[i];
        t17 = t16 * b[i+1];
        t18 = t17 - c[i-1];
        t19 = t18 + a[i-1];
        
        /* Create conditional basic block inside loop */
        if (t5 & 1) {
            /* Path 1: More computations */
            t19 = t19 * 3;
            t19 = t19 + high_latency_op(t19, invariant);
            result += t19;
        } else {
            /* Path 2: Different computations */
            t19 = t19 / 2;
            t19 = t19 - high_latency_op(t19, invariant);
            result += t19;
        }
        
        /* Cross-iteration dependency */
        a[i] = t19 % 256;  /* Modulo creates longer latency */
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int* a, int* b, int* c, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main(void) {
    const int N = 1024;
    int* a = malloc(N * sizeof(int));
    int* b = malloc(N * sizeof(int));
    int* c = malloc(N * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic values */
    init_arrays(a, b, c, N);
    
    /* Call hot function multiple times to ensure execution */
    int total = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = 17 + (iter % 5);
        total += target_loop(a, b, c, N);
    }
    
    printf("Result: %d\n", total);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
