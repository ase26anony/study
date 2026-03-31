/* test_modulo_sched.c
 * Designed to trigger modulo scheduling move edge logging (lines 596-606 in modulo-sched.cc)
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -fdump-rtl-sms -fdump-rtl-sms-details test_modulo_sched.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Global invariant value to create cross-iteration dependencies */
int GLOBAL_INVARIANT = 7;

/* High-latency operation to encourage move insertion */
__attribute__((noinline, cold)) 
int high_latency_op(int x) {
    /* Division has higher latency than basic arithmetic */
    return x % 13;  /* Non-trivial modulo operation */
}

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant value from global */
    int invariant = GLOBAL_INVARIANT;
    
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        
        /* Chain of dependent operations with array accesses */
        t0 = a[i] + invariant;           /* Use invariant in chain */
        t1 = t0 * b[i];
        t2 = t1 - c[i];
        t3 = t2 + a[i+1];                /* Access with offset */
        t4 = t3 * b[i-1];                /* Another offset access */
        t5 = high_latency_op(t4);        /* High-latency operation */
        t6 = t5 + t0;                    /* Cross-dependency */
        t7 = t6 * invariant;             /* Use invariant again */
        t8 = t7 - t2;
        t9 = t8 + t4;
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More computations */
            t9 = t9 * 3;
            result += t9 - t1;
        } else {
            /* Path 2: Different computations */
            t9 = t9 / 2;
            result += t9 + t3;
        }
        
        /* Additional dependent operations after conditional */
        int t10 = t9 * a[i];
        int t11 = t10 + b[i];
        int t12 = t11 - c[i];
        int t13 = t12 * invariant;       /* Another invariant use */
        int t14 = high_latency_op(t13);  /* Another high-latency op */
        
        result ^= t14;  /* Mix result */
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7fffffff;
        a[i] = (seed >> 16) & 0xFF;
        b[i] = (seed >> 8) & 0xFF;
        c[i] = seed & 0xFF;
    }
}

int main() {
    const int SIZE = 1024;
    int* a = malloc(SIZE * sizeof(int));
    int* b = malloc(SIZE * sizeof(int));
    int* c = malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic values */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    int total_result = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT = 7 + (iter & 3);
        total_result += compute_loop(a, b, c, SIZE);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
