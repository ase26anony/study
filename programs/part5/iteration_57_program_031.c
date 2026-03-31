/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 10

/* Global variables to create loop invariants */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function with higher latency */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division creates higher latency operation */
    return (x % y) ? (x / y) : (x * y);
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int target_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
        
        /* Start chain with loop-invariant values (creates move edges) */
        t0 = g_invariant1 + a[i];
        t1 = t0 * g_invariant2;
        t2 = t1 - b[i];
        
        /* Array accesses with offsets creating memory dependencies */
        t3 = a[i-1] + t2;
        t4 = b[i+1] * t3;
        t5 = c[i] - t4;
        
        /* High latency operation in the chain */
        t6 = high_latency_op(t5, g_invariant3);
        
        /* Continue dependency chain */
        t7 = t6 + a[i];
        t8 = t7 * b[i];
        t9 = t8 - c[i];
        t10 = t9 + a[i+1];
        t11 = t10 * b[i-1];
        t12 = t11 - c[i+1];
        
        /* Conditional to create multiple basic blocks */
        if (t12 & 1) {
            /* Path 1: More computations */
            t13 = t12 * g_invariant1;
            t14 = t13 + high_latency_op(t13, 3);
            t15 = t14 - g_invariant2;
            result += t15;
        } else {
            /* Path 2: Different computations */
            t13 = t12 / g_invariant2;
            t14 = t13 * high_latency_op(t13, 5);
            t15 = t14 + g_invariant3;
            result -= t15;
        }
        
        /* Cross-iteration dependency */
        a[i] = t15 & 0xFF;
    }
    
    return result;
}

/* Initialize arrays with deterministic pseudo-random values */
void init_arrays(int* a, int* b, int* c, int n) {
    int seed = 42;
    for (int i = 0; i < n; i++) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        a[i] = seed % 100;
        b[i] = (seed >> 8) % 100;
        c[i] = (seed >> 16) % 100;
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
        g_invariant1 = (g_invariant1 * 3) % 17;
        g_invariant2 = (g_invariant2 * 5) % 19;
        g_invariant3 = (g_invariant3 * 7) % 23;
        
        total_result += target_loop(a, b, c, SIZE);
    }
    
    printf("Result: %d\n", total_result);
    return 0;
}
