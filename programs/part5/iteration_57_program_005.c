/* test_modulo_sched.c - Target for modulo scheduler coverage */

#include <stdio.h>
#include <stdlib.h>

/* Global variable to create loop-invariant value */
int GLOBAL_INVARIANT = 12345;

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 123456789;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

/* Hot function with complex loop for modulo scheduling */
__attribute__((hot, noinline))
int compute_loop(int* a, int* b, int* c, int* d, int n) {
    int result = 0;
    int invariant = GLOBAL_INVARIANT;  /* Loop-invariant value */
    
    /* Main loop with high register pressure and dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* Many scalar temporaries creating register pressure */
        int t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
        int t10, t11, t12, t13, t14, t15, t16, t17, t18, t19;
        
        /* Chain of dependent operations with different latencies */
        t0 = a[i] + invariant;          /* Uses invariant across iterations */
        t1 = t0 * b[i];                 /* Multiplication (higher latency) */
        t2 = t1 - c[i];
        t3 = t2 / 7;                    /* Division (high latency) */
        t4 = t3 + a[i-1];
        t5 = t4 * b[i+1];
        t6 = t5 % 13;                   /* Modulo (higher latency) */
        t7 = t6 - c[i-1];
        t8 = t7 * invariant;            /* Another use of invariant */
        t9 = t8 + d[i];
        
        /* More operations to extend dependency chain */
        t10 = t9 * 3;
        t11 = t10 - a[i];
        t12 = t11 / 5;                  /* Another division */
        t13 = t12 + b[i];
        t14 = t13 * c[i];
        t15 = t14 % 17;                 /* Another modulo */
        t16 = t15 - d[i];
        t17 = t16 * invariant;          /* Third use of invariant */
        t18 = t17 + t0;                 /* Cross-iteration dependency hint */
        t19 = t18 - t9;
        
        /* Conditional to create multiple basic blocks */
        if (t19 & 1) {
            /* Path 1: More computations */
            t19 = t19 * 2 + 1;
            t19 = t19 / 3;              /* Division in conditional path */
            result += t19;
        } else {
            /* Path 2: Different computations */
            t19 = t19 * 3 - 1;
            t19 = t19 % 11;             /* Modulo in conditional path */
            result ^= t19;
        }
        
        /* Additional invariant usage after conditional */
        result += (invariant & 0xFF);
    }
    
    return result;
}

/* Cold function to prevent inlining of high-latency operations */
__attribute__((noinline, cold))
int cold_division(int x, int y) {
    /* Force function call instead of inline division */
    return x / y;
}

__attribute__((noinline, cold))
int cold_modulo(int x, int y) {
    /* Force function call instead of inline modulo */
    return x % y;
}

/* Another hot function with different pattern */
__attribute__((hot, noinline))
int compute_loop2(int* a, int* b, int* c, int n) {
    int result = 0;
    int inv1 = GLOBAL_INVARIANT;
    int inv2 = GLOBAL_INVARIANT * 2;
    
    for (int i = 2; i < n - 2; i++) {
        /* Different dependency pattern */
        int v0 = a[i] + inv1;
        int v1 = cold_division(v0, 7);      /* High latency call */
        int v2 = v1 * b[i];
        int v3 = cold_modulo(v2, 13);       /* Another high latency call */
        int v4 = v3 - c[i];
        int v5 = v4 + inv2;
        int v6 = v5 * a[i+1];
        int v7 = v6 - b[i-1];
        int v8 = cold_division(v7, 3);
        int v9 = v8 + inv1;
        
        /* Complex conditional with both paths having computations */
        if ((v9 + i) & 2) {
            int t = v9 * v0;
            result += cold_modulo(t, 19);
        } else {
            int t = v9 - v0;
            result ^= cold_division(t, 5);
        }
        
        /* Use invariant in memory address calculation */
        result += a[(i + inv1) % n];
    }
    
    return result;
}

int main(void) {
    const int N = 1024;
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(N * sizeof(int));
    int *d = malloc(N * sizeof(int));
    
    /* Initialize arrays with deterministic pseudo-random values */
    for (int i = 0; i < N; i++) {
        a[i] = lcg_rand() % 1000;
        b[i] = lcg_rand() % 1000;
        c[i] = lcg_rand() % 1000;
        d[i] = lcg_rand() % 1000;
    }
    
    int total = 0;
    
    /* Call hot functions multiple times to ensure execution */
    for (int iter = 0; iter < 10; iter++) {
        total += compute_loop(a, b, c, d, N);
        total += compute_loop2(a, b, c, N);
        
        /* Modify invariant slightly each iteration */
        GLOBAL_INVARIANT += iter;
    }
    
    printf("Result: %d\n", total);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
