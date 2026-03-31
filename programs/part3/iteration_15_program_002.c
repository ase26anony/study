/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int* restrict a, int* restrict b, 
                        int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        /* Also creates output dependence on b[i] from previous statement */
        b[i] = a[i] - d[i];
        
        /* Additional operation to create more edges in the DDG */
        c[i] = (b[i] << 2) + i;
        
        /* Create anti-dependence: d[i] read, then written in next iteration? */
        /* Actually create output dependence on d[i] within same iteration */
        d[i] = d[i] + a[i] * 2;
    }
    
    /* Return checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop variant with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int* restrict x, int* restrict y, 
                         int* restrict z, int n) {
    int i;
    
    /* Different pattern: chain of dependencies */
    for (i = 2; i < n; ++i) {
        /* Multiple true dependencies in a chain */
        x[i] = x[i-1] + y[i-2];
        y[i] = x[i] * z[i];
        z[i] = y[i] - x[i-1];
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i;
    int checksum1, checksum2;
    
    /* Dynamically allocate arrays to avoid stack overflow with large N */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    int* d = (int*)malloc(N * sizeof(int));
    
    int* x = (int*)malloc(N * sizeof(int));
    int* y = (int*)malloc(N * sizeof(int));
    int* z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 29) % 983;
        
        x[i] = (i * 41) % 977;
        y[i] = (i * 59) % 971;
        z[i] = (i * 67) % 967;
    }
    
    /* Set initial values to create proper dependencies */
    a[0] = 1;
    b[0] = 2;
    c[0] = 3;
    d[0] = 4;
    
    x[0] = x[1] = 5;
    y[0] = y[1] = 6;
    z[0] = z[1] = 7;
    
    /* Call the loop functions - these should trigger DDG construction */
    checksum1 = compute_loop(a, b, c, d, N);
    checksum2 = compute_loop2(x, y, z, N);
    
    /* Use results to prevent dead code elimination */
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 2: %d\n", checksum2);
    printf("Total: %d\n", checksum1 + checksum2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
