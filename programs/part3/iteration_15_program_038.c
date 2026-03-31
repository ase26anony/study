/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int GLOBAL_N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create a non-trivial DDG */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, but b[i] is read in next iteration's 
           a[i-1] * b[i] expression (through the array 'b') */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more edges in the DDG */
        c[i] = (b[i-1] << 2) + i;  /* Anti-dependence on b[i-1] */
        d[i] = d[i-1] + a[i] * 3;  /* True dependence on d[i-1] */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with output dependencies */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    for (i = 1; i < n; ++i) {
        /* Output dependence: x[i] written twice in same iteration */
        x[i] = y[i] * 2;
        x[i] = x[i] + x[i-1];  /* Also has true dependence on x[i-1] */
        
        /* Complex expression to create more RTL instructions */
        y[i] = (x[i] << 1) | (y[i-1] & 0xFF);
    }
    
    return x[n-1] * y[n-1];
}

int main(void) {
    int i, result1, result2;
    int n = GLOBAL_N;  /* Use volatile variable to prevent constant folding */
    
    /* Allocate and initialize arrays with deterministic values */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    int *c = (int*)malloc(n * sizeof(int));
    int *d = (int*)malloc(n * sizeof(int));
    int *x = (int*)malloc(n * sizeof(int));
    int *y = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < n; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        x[i] = (i * 101) % 977;
        y[i] = (i * 113) % 971;
    }
    
    /* Call the loops to force DDG construction during compilation */
    result1 = compute_loop(a, b, c, d, n);
    result2 = compute_loop2(x, y, n);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Use results to prevent optimization */
    if (result1 > 1000000 || result2 > 1000000) {
        printf("Unexpected large results\n");
    }
    
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
