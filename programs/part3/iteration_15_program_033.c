/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int *restrict a, int *restrict b, 
                 int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more dependencies */
        c[i] = b[i-1] + i;      /* Anti-dependence on b[i-1] */
        d[i] = c[i] << 2;       /* Flow dependence within same iteration */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with output dependencies */
__attribute__((noinline))
int compute_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    for (i = 0; i < n; ++i) {
        /* Output dependence: x[i] written twice in same iteration */
        x[i] = y[i] * 3;
        x[i] = x[i] + (i & 0xFF);  /* Overwrites previous value */
        
        /* Flow dependence across iterations */
        y[i] = (i > 0) ? x[i-1] + y[i-1] : 1;
    }
    
    return x[n-1] + y[n-1];
}

int main(void) {
    int i, result1, result2;
    
    /* Allocate arrays with restrict to help alias analysis */
    int *restrict a = (int*)malloc(N * sizeof(int));
    int *restrict b = (int*)malloc(N * sizeof(int));
    int *restrict c = (int*)malloc(N * sizeof(int));
    int *restrict d = (int*)malloc(N * sizeof(int));
    int *restrict x = (int*)malloc(N * sizeof(int));
    int *restrict y = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        x[i] = (i * 101) % 977;
        y[i] = (i * 113) % 971;
    }
    
    /* Call loops with dependencies to trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, N);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to prevent optimization */
    if (result1 > 1000000 || result2 > 1000000) {
        printf("Unexpected large result\n");
    }
    
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
