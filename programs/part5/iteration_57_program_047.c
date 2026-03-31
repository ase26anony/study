/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create loop-invariant values */
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
int compute_hot_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = g_invariant1;
    int inv2 = g_invariant2;
    int inv3 = g_invariant3;
    
    /* Main loop with high register pressure */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating dependency chain */
        int t0 = a[i];
        int t1 = t0 + inv1;           /* Use invariant - creates potential move */
        int t2 = b[i] * t1;
        int t3 = t2 - c[i];
        int t4 = t3 + a[i-1];         /* Array access with offset */
        int t5 = t4 * inv2;           /* Another invariant use */
        int t6 = b[i+1] + t5;         /* Forward array access */
        int t7 = t6 - inv3;
        
        /* High latency operation in the chain */
        int t8 = high_latency_op(t7, t1);
        
        int t9 = t8 + a[i];
        int t10 = t9 * b[i];
        int t11 = t10 - c[i-1];
        int t12 = t11 + inv1;
        int t13 = t12 * t3;
        int t14 = t13 - inv2;
        int t15 = t14 + b[i+1];
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* Path 1: More computations */
            int t16 = t15 * inv3;
            int t17 = t16 + a[i];
            int t18 = high_latency_op(t17, 3);
            result += t18;
        } else {
            /* Path 2: Different computations */
            int t16 = t15 + inv1;
            int t17 = t16 * t8;
            int t18 = t17 - inv2;
            result += t18;
        }
        
        /* Cross-iteration dependency */
        a[i] = t15;  /* Store creates loop-carried dependency */
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
    
    /* Call hot function multiple times to ensure it's compiled */
    int total = 0;
    for (int iter = 0; iter < 10; iter++) {
        /* Modify invariants slightly each iteration */
        g_invariant1 = (g_invariant1 * 3) % 17;
        g_invariant2 = (g_invariant2 * 5) % 23;
        g_invariant3 = (g_invariant3 * 7) % 29;
        
        total += compute_hot_loop(a, b, c, N);
    }
    
    printf("Result: %d\n", total);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
