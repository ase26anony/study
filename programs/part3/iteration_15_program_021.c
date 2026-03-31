/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create a non-trivial DDG */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then b[i-1] might be read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Add more operations to create different dependency types */
        c[i] = (c[i-1] << 1) + b[i];  /* Another flow dependence on c[i-1] */
        d[i] = a[i] * 2 - b[i-1];     /* Anti-dependence on b[i-1] */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with output dependencies */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Loop with output dependencies within same iteration */
    for (i = 0; i < n; ++i) {
        x[i] = i * 3;
        x[i] = x[i] + y[i];  /* Output dependence on x[i] */
        y[i] = x[i] * 2;
        y[i] = y[i] - i;     /* Output dependence on y[i] */
    }
    
    return x[n-1] + y[n-1];
}

int main(void) {
    int i;
    int checksum1, checksum2;
    
    /* Dynamically allocate arrays to avoid stack overflow with large N */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1003;
        c[i] = (i * 71) % 1007;
        d[i] = (i * 89) % 1013;
        x[i] = (i * 101) % 1021;
        y[i] = (i * 103) % 1031;
    }
    
    /* Call the loops to force DDG construction */
    checksum1 = compute_loop(a, b, c, d, N);
    checksum2 = compute_loop2(x, y, N);
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 2: %d\n", checksum2);
    printf("Total: %d\n", checksum1 + checksum2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
