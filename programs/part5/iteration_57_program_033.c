/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop-invariant values */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold)) 
int slow_operation(int x, int y) {
    /* Force a higher latency operation */
    return (x % y) + (x / (y | 1));
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = g_invariant1;
    int inv2 = g_invariant2;
    int inv3 = g_invariant3;
    
    /* Main loop with high register pressure */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + inv1;           /* Use invariant */
        int t2 = t1 * b[i];
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];         /* Array access with offset */
        int t5 = t4 * inv2;           /* Another invariant */
        int t6 = t5 - b[i+1];         /* Forward array access */
        int t7 = t6 / (inv3 | 1);     /* Division for higher latency */
        int t8 = t7 + t0;             /* Cross-dependency */
        int t9 = t8 * t3;
        int t10 = t9 - t5;
        int t11 = t10 + t2;
        int t12 = t11 * t7;
        
        /* Conditional to create multiple basic blocks */
        if (t12 & 1) {
            /* Path 1: More computations */
            int t13 = slow_operation(t12, inv1);  /* Higher latency call */
            int t14 = t13 + a[i];
            int t15 = t14 * b[i-1];
            result += t15;
        } else {
            /* Path 2: Different computations */
            int t13 = t12 + inv2;
            int t14 = t13 * c[i];
            int t15 = t14 - inv3;
            int t16 = slow_operation(t15, inv2);  /* Another call */
            result += t16;
        }
        
        /* Additional computations to increase critical path */
        int t17 = t12 + result;
        int t18 = t17 * inv1;
        int t19 = t18 - t3;
        int t20 = t19 + t6;
        int t21 = t20 * t8;
        int t22 = t21 / (inv2 | 1);
        result ^= t22;  /* Mix result */
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
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    int total_result = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3) % 17;
        g_invariant2 = (g_invariant2 * 5) % 23;
        g_invariant3 = (g_invariant3 * 7) % 29;
        
        total_result += compute_hot_loop(a, b, c, SIZE);
        
        /* Rotate arrays to change data dependencies */
        int temp = a[0];
        for (int i = 0; i < SIZE - 1; i++) {
            a[i] = a[i + 1];
        }
        a[SIZE - 1] = temp;
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
