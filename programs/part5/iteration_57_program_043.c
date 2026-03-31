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
int slow_operation(int x, int y) {
    /* Division has higher latency than basic arithmetic */
    return (x % (y | 1)) + 1;  /* Ensure no division by zero */
}

/* Hot function containing the target loop */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int n) {
    int result = 0;
    
    /* Loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i];
        int t1 = t0 + b[i];          /* Depends on t0 */
        int t2 = t1 * g_invariant1;  /* Uses invariant */
        int t3 = t2 - c[i];          /* Depends on t2 */
        int t4 = t3 + a[i-1];        /* Array access with offset */
        int t5 = t4 * b[i+1];        /* Array access with offset */
        int t6 = t5 / g_invariant2;  /* Division with invariant */
        int t7 = t6 + t3;            /* Cross-dependency */
        int t8 = t7 * t1;            /* More dependencies */
        int t9 = t8 - g_invariant3;  /* Another invariant */
        int t10 = t9 + a[i+1];       /* Forward array access */
        
        /* Higher latency operation in the chain */
        int t11 = slow_operation(t10, t8);
        
        int t12 = t11 + t5;
        int t13 = t12 * t7;
        int t14 = t13 - t2;
        int t15 = t14 + t9;
        int t16 = t15 * t11;
        int t17 = t16 / (g_invariant1 | 1);
        int t18 = t17 + t13;
        int t19 = t18 - t15;
        int t20 = t19 * t16;
        
        /* Conditional to create multiple basic blocks */
        if (t20 & 1) {
            /* Path 1: More computations */
            int t21 = t20 + t17;
            int t22 = t21 * g_invariant2;
            int t23 = t22 - t19;
            result += t23;
        } else {
            /* Path 2: Different computations */
            int t21 = t20 - t17;
            int t22 = t21 / (g_invariant3 | 1);
            int t23 = t22 + t19;
            result += t23;
        }
        
        /* Additional computations after the conditional */
        int t24 = result * t20;
        result = t24 % 1000;  /* Keep result bounded */
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
        
        total_result += compute_loop(a, b, c, SIZE);
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
