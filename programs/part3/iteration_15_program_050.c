/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variable to prevent constant propagation */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int* restrict a, int* restrict b, 
                        int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create a complex DDG */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written here, but b[i-1] was read in previous iteration */
        /* Also flow dependence: b[i] depends on a[i] computed above */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more edges in the DDG */
        c[i] = (b[i-1] << 2) + i;      /* Anti-dependence on b[i-1] */
        d[i] = (a[i] & 0xFF) | (d[i-1] & 0xFF00); /* Flow on d[i-1], output on d[i] */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int* restrict x, int* restrict y, 
                         int* restrict z, int n) {
    int i;
    
    /* Different loop with output dependencies and multiple operations */
    for (i = 1; i < n; ++i) {
        /* Multiple writes to same array in same iteration (output dependence) */
        x[i] = y[i] * z[i];
        x[i] = x[i] + x[i-1];  /* Overwrites x[i] - output dependence */
        
        /* Chain of dependencies within iteration */
        y[i] = x[i] << 1;
        z[i] = y[i] + z[i-1];  /* Flow dependence on z[i-1] */
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i, result1, result2;
    
    /* Use volatile to get actual N value at runtime */
    int n = N;
    
    /* Allocate arrays with restrict to help alias analysis */
    int* restrict a = (int*)malloc(n * sizeof(int));
    int* restrict b = (int*)malloc(n * sizeof(int));
    int* restrict c = (int*)malloc(n * sizeof(int));
    int* restrict d = (int*)malloc(n * sizeof(int));
    
    int* restrict x = (int*)malloc(n * sizeof(int));
    int* restrict y = (int*)malloc(n * sizeof(int));
    int* restrict z = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < n; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 73) % 1001;
        c[i] = (i * 101) % 1001;
        d[i] = (i * 137) % 1001;
        
        x[i] = (i * 53) % 1001;
        y[i] = (i * 89) % 1001;
        z[i] = (i * 113) % 1001;
    }
    
    /* Call the loops - each should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, n);
    result2 = compute_loop2(x, y, z, n);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
