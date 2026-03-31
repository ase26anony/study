/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Volatile variable to prevent constant propagation */
volatile int g_N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more DDG edges */
        c[i] = b[i-1] + d[i];  /* Anti-dependence on b[i-1] */
        d[i] = a[i] << 2;      /* Output dependence on d[i] within iteration */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop variant with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, 
                         int *restrict z, int n) {
    int i;
    
    /* Different dependency pattern to exercise more DDG edge types */
    for (i = 2; i < n; ++i) {
        /* Multiple true dependencies */
        x[i] = x[i-1] + x[i-2];
        
        /* Chain of dependencies */
        y[i] = x[i] * y[i-1];
        
        /* Independent operation that creates output dependence */
        z[i] = z[i] + (y[i] >> 1);
    }
    
    return x[n-1] * y[n-1] + z[n-1];
}

int main(void) {
    int N = g_N;  /* Use volatile to get actual value */
    int checksum1, checksum2;
    
    /* Allocate and initialize arrays with deterministic values */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int *z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        
        x[i] = (i * 101) % 977;
        y[i] = (i * 103) % 971;
        z[i] = (i * 107) % 967;
    }
    
    /* Call the loops to force DDG construction during compilation */
    checksum1 = compute_loop(a, b, c, d, N);
    checksum2 = compute_loop2(x, y, z, N);
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 2: %d\n", checksum2);
    printf("Total: %d\n", checksum1 + checksum2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
