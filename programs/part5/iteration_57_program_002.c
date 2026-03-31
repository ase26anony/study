/* test_modulo_sched.c - Target program for modulo scheduler coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to create loop-invariant values */
int global_invariant1 = 12345;
int global_invariant2 = 67890;
int global_invariant3 = 54321;

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Complex operation that won't be inlined */
    return (x * y) ^ (x + y) ^ (x - y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int target_loop(int* a, int* b, int* c, int* d, int n) {
    int result = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = global_invariant1;
    int inv2 = global_invariant2;
    int inv3 = global_invariant3;
    
    /* Main loop with high register pressure and dependencies */
    for (int i = 2; i < n - 2; i++) {
        /* Many scalar temporaries creating long dependency chain */
        int t0 = a[i];
        int t1 = t0 + inv1;           /* Use invariant */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 ^ inv2;           /* Use another invariant */
        int t5 = t4 + a[i-1];         /* Array access with offset */
        int t6 = t5 * b[i+1];         /* Different offset */
        int t7 = t6 - c[i-1];
        int t8 = t7 ^ d[i];
        int t9 = t8 + inv3;
        
        /* High latency operation in the chain */
        int t10 = high_latency_op(t9, inv1);
        
        /* More dependent operations */
        int t11 = t10 + a[i+1];
        int t12 = t11 * b[i-1];
        int t13 = t12 - c[i+1];
        int t14 = t13 ^ d[i-1];
        int t15 = t14 + t0;           /* Loop-carried dependency pattern */
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 * 3;
            int t17 = t16 + inv2;
            int t18 = high_latency_op(t17, t5);  /* Another high latency op */
            int t19 = t18 ^ inv3;
            result += t19;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 / 2;        /* Division - potentially higher latency */
            int t17 = t16 - inv1;
            int t18 = t17 * t3;
            int t19 = t18 | inv2;
            result += t19;
        }
        
        /* Cross-iteration dependency through array */
        d[i] = t15;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int* d, int n) {
    unsigned int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = seed * 1103515245 + 12345;
        a[i] = (seed >> 16) & 0x7FFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
        d[i] = 0;
    }
}

int main() {
    const int SIZE = 1024;
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    int* d = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic values */
    init_arrays(a, b, c, d, SIZE);
    
    int total_result = 0;
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariants slightly each iteration */
        global_invariant1 += iter;
        global_invariant2 -= iter;
        global_invariant3 ^= iter;
        
        total_result += target_loop(a, b, c, d, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
