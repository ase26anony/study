/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create loop-invariant values */
int g_invariant1 = 7;
int g_invariant2 = 13;
int g_invariant3 = 19;

/* Non-inlineable function to create higher latency operations */
__attribute__((noinline, cold)) 
int high_latency_op(int x, int y) {
    /* Division creates higher latency */
    return (x % y) ? x : y;
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop-invariant values from globals */
    int inv1 = g_invariant1;
    int inv2 = g_invariant2;
    int inv3 = g_invariant3;
    
    /* Main loop with high register pressure */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i] + inv1;          /* Uses invariant */
        int t1 = t0 * b[i];
        int t2 = t1 - c[i];
        int t3 = t2 + a[i-1];          /* Array access with offset */
        int t4 = t3 * inv2;            /* Uses another invariant */
        int t5 = t4 - b[i+1];          /* Forward array access */
        
        /* Conditional to create multiple basic blocks */
        if (t5 & 1) {
            /* Path 1: More dependent operations */
            int t6 = high_latency_op(t5, inv3);  /* Higher latency op */
            int t7 = t6 + c[i-1];
            int t8 = t7 * a[i];
            int t9 = t8 - t2;          /* Cross-path dependency */
            result += t9;
        } else {
            /* Path 2: Different chain but still dependent */
            int t6 = t5 * 3;
            int t7 = t6 + high_latency_op(t6, 5); /* Mixed latency */
            int t8 = t7 - b[i];
            int t9 = t8 / 2;           /* Division for latency */
            result += t9;
        }
        
        /* More operations after conditional */
        int t10 = result & 0xFF;
        int t11 = t10 * inv1;
        int t12 = t11 + a[i];
        int t13 = t12 - t5;
        int t14 = t13 * 2;
        
        /* Use result to prevent dead code elimination */
        result ^= t14;
    }
    
    return result;
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int sum = 0;
    int inv = g_invariant1 + g_invariant2;
    
    for (int i = 0; i < n; i++) {
        /* Long dependency chain */
        int v0 = a[i] + inv;
        int v1 = v0 * b[i % n];
        int v2 = v1 - c[i % n];
        int v3 = v2 + a[(i + 1) % n];
        int v4 = v3 * inv;
        int v5 = v4 - b[(i + 2) % n];
        int v6 = v5 + c[(i + 3) % n];
        int v7 = v6 * 3;
        int v8 = v7 - inv;
        int v9 = v8 / 4;
        
        /* Conditional with both paths having computations */
        if (v9 > 0) {
            int w0 = high_latency_op(v9, 17);
            int w1 = w0 + a[i % n];
            sum += w1;
        } else {
            int w0 = v9 * 5;
            int w1 = w0 - b[i % n];
            sum += w1;
        }
        
        /* Continue dependency chain */
        int x0 = sum & 255;
        int x1 = x0 * v9;
        sum = x1 ^ sum;
    }
    
    return sum;
}

/* Simple deterministic RNG for array initialization */
static unsigned int seed = 123456789;
unsigned int simple_rand() {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

int main() {
    const int N = 1024;
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    
    /* Initialize arrays with deterministic pseudo-random values */
    for (int i = 0; i < N; i++) {
        a[i] = simple_rand() % 1000;
        b[i] = simple_rand() % 1000;
        c[i] = simple_rand() % 1000;
    }
    
    int total = 0;
    
    /* Call hot functions multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        total += compute_loop(a, b, c, N);
        total += compute_loop2(a, b, c, N);
        
        /* Modify arrays slightly to vary computation */
        for (int i = 0; i < N; i++) {
            a[i] = (a[i] + 1) % 1000;
        }
    }
    
    printf("Result: %d\n", total);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
