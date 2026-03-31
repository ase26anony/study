/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Global variables to create loop invariants */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division has higher latency than basic arithmetic */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating long dependency chain */
        int t0 = a[i] + g_invariant1;          /* Use invariant */
        int t1 = t0 * b[i];
        int t2 = t1 - c[i];
        int t3 = t2 + a[i-1];                  /* Array access with offset */
        int t4 = t3 * g_invariant2;            /* Another invariant */
        int t5 = high_latency_op(t4, g_invariant3); /* Higher latency op */
        int t6 = t5 + b[i+1];                  /* Another offset access */
        int t7 = t6 * t0;                      /* Cross-dependent */
        int t8 = t7 - c[i-1];
        int t9 = t8 + t2;
        int t10 = t9 * t5;
        
        /* Additional temporaries to increase register pressure */
        int t11 = t10 + a[i] * b[i];
        int t12 = t11 - c[i] / 3;
        int t13 = t12 * t7;
        int t14 = t13 + t3;
        int t15 = t14 - t8;
        int t16 = t15 * t11;
        int t17 = t16 + t9;
        int t18 = t17 - t12;
        int t19 = t18 * t13;
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* Path 1: More computations */
            int t20 = t19 + high_latency_op(t14, 5);
            int t21 = t20 * t16;
            int t22 = t21 - t17;
            result += t22 + t10;
        } else {
            /* Path 2: Different computations but still dependent */
            int t20 = t19 - high_latency_op(t15, 3);
            int t21 = t20 + t18;
            int t22 = t21 * t14;
            result += t22 - t11;
        }
        
        /* More operations after the conditional */
        int t23 = result * t19;
        int t24 = t23 + g_invariant1;          /* Another invariant use */
        int t25 = high_latency_op(t24, 2);
        result = t25 ^ result;
        
        /* Additional array accesses with different offsets */
        result += a[i+1] - b[i-1] + c[i+1];
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
    int a[SIZE], b[SIZE], c[SIZE];
    int total_result = 0;
    
    /* Initialize arrays */
    init_arrays(a, b, c, SIZE);
    
    /* Call hot function multiple times to ensure it's compiled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3 + 1) & 0xFF;
        g_invariant2 = (g_invariant2 * 5 + 2) & 0xFF;
        g_invariant3 = (g_invariant3 * 7 + 3) & 0xFF;
        
        total_result += compute_loop(a, b, c, SIZE);
        
        /* Slightly modify arrays to prevent complete optimization */
        a[iter % SIZE] = iter;
        b[(iter + 1) % SIZE] = iter * 2;
        c[(iter + 2) % SIZE] = iter * 3;
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
