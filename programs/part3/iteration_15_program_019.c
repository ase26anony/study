/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int GLOBAL_N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int *restrict a, int *restrict b, 
                 int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies for DDG construction */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then b[i-1] might be read in next iteration
           if we had: c[i] = b[i-1] + ... (we'll add this below) */
        
        /* Output dependence: multiple writes to same array element in same iteration */
        a[i] = a[i] + d[i];  /* Second write to a[i] creates output dependence */
        
        /* Another true dependence chain with b array */
        b[i] = a[i] - d[i];
        
        /* Anti-dependence: read b[i-1] after it was written in previous iteration */
        c[i] = b[i-1] + i;
    }
    
    /* Return checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
int compute_loop2(int *restrict x, int *restrict y, 
                  int *restrict z, int n) {
    int i;
    
    /* Loop with shift operations and complex dependencies */
    for (i = 2; i < n; ++i) {
        /* Multiple interleaved dependencies */
        x[i] = x[i-1] + y[i-2];      /* True dependence with distance 1 and 2 */
        y[i] = x[i] << 2;            /* True dependence within same iteration */
        z[i] = z[i-1] - y[i];        /* True dependence on z, anti on y */
        
        /* Create register pressure with multiple operations */
        int temp = x[i] * 3;
        y[i] = y[i] + temp;          /* Output dependence on y[i] */
        z[i] = z[i] + (temp >> 1);   /* Output dependence on z[i] */
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int N = GLOBAL_N;  /* Use volatile variable to prevent constant folding */
    int i, result1, result2;
    
    /* Allocate and initialize arrays with restrict to help alias analysis */
    int *restrict a = (int*)malloc(N * sizeof(int));
    int *restrict b = (int*)malloc(N * sizeof(int));
    int *restrict c = (int*)malloc(N * sizeof(int));
    int *restrict d = (int*)malloc(N * sizeof(int));
    int *restrict x = (int*)malloc(N * sizeof(int));
    int *restrict y = (int*)malloc(N * sizeof(int));
    int *restrict z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
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
        y[i] = (i * 107) % 971;
        z[i] = (i * 113) % 967;
    }
    
    /* Call the loop functions - these should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
