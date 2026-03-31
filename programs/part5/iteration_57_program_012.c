/* test_modulo_sched.c */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 100

/* Global variables to create loop invariants */
int g_inv1 = 12345;
int g_inv2 = 67890;
int g_inv3 = 54321;

/* High-latency operations marked noinline to prevent inlining */
__attribute__((noinline, cold)) int slow_operation(int x, int y) {
    /* Simulate higher latency operation */
    return (x % (y | 1)) + (y % (x | 1));
}

__attribute__((noinline, cold)) int another_slow_op(int x) {
    /* Another potentially high-latency operation */
    return (x * 13) % 17;
}

/* Hot function containing the target loop */
__attribute__((hot, noinline)) 
int compute_hot_loop(int* a, int* b, int* c, int n, int inv1, int inv2) {
    int result = 0;
    
    /* Main loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries with dependencies */
        int t0 = a[i] + inv1;           /* Loop invariant used */
        int t1 = t0 * b[i];
        int t2 = t1 - c[i];
        int t3 = t2 + a[i-1];           /* Array access with offset */
        int t4 = t3 * inv2;             /* Another loop invariant */
        int t5 = t4 / (b[i+1] | 1);     /* Array access with offset + division */
        
        /* Mix with high-latency operations */
        int t6 = slow_operation(t5, g_inv3);  /* Global invariant + function call */
        int t7 = t6 + t2;                     /* Cross-dependency */
        int t8 = another_slow_op(t7);         /* Another function call */
        int t9 = t8 * t4;
        int t10 = t9 - t1;
        int t11 = t10 + c[i-1];               /* Another offset access */
        int t12 = t11 % (a[i+1] | 1);         /* Modulo operation */
        int t13 = t12 * t5;
        int t14 = t13 + t8;
        int t15 = t14 - t3;
        
        /* Conditional to create multiple basic blocks */
        if (t15 & 1) {
            /* True path with more computations */
            int t16 = t15 * 3;
            int t17 = slow_operation(t16, t10);
            int t18 = t17 + t12;
            result += t18;
        } else {
            /* False path with different computations */
            int t16 = t15 / 2;
            int t17 = another_slow_op(t16);
            int t18 = t17 - t14;
            result += t18;
        }
        
        /* Additional computations after the conditional */
        int t19 = result * 7;
        result = t19 % 1000;
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
    for (int iter = 0; iter < ITERS; iter++) {
        /* Modify invariants slightly each iteration */
        g_inv1 = (g_inv1 * 13 + 17) & 0xFFF;
        g_inv2 = (g_inv2 * 29 + 31) & 0xFFF;
        g_inv3 = (g_inv3 * 47 + 53) & 0xFFF;
        
        total_result += compute_hot_loop(a, b, c, SIZE, g_inv1, g_inv2);
    }
    
    printf("Final result: %d\n", total_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    
    return 0;
}
