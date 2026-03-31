/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int* restrict a, int* restrict b, 
                 int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, but b[i-1] might be read in next iteration
           if we had b[i-1] usage. We'll create this separately. */
        b[i] = a[i] - d[i];
        
        /* Create anti-dependence explicitly: use b[i-1] */
        c[i] = b[i-1] + i;
        
        /* Output dependence: multiple writes to d[i] in same iteration */
        d[i] = a[i] * 2;
        d[i] = d[i] + 1;  /* Second write creates output dependence */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another function with different dependency patterns */
__attribute__((noinline))
int compute_loop2(int* restrict x, int* restrict y, int* restrict z, int n) {
    int i;
    
    /* Different loop with shift operations */
    for (i = 2; i < n; ++i) {
        /* Complex carried dependencies */
        x[i] = x[i-1] + y[i-2] * z[i];
        y[i] = x[i] << 2;
        z[i] = y[i-1] - x[i-2];
    }
    
    return x[n-1] + y[n-1];
}

int main(void) {
    int i;
    int result1, result2;
    
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
        y[i] = (i * 43) % 971;
        z[i] = (i * 47) % 967;
    }
    
    /* Call the loop functions - these should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
