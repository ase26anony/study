/* test_ddg.c - Program to trigger GCC's DDG edge creation for modulo scheduling */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int GLOBAL_N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create a complex DDG */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        /* Also output dependence: b[i] is written multiple times in same iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operation to create more dependencies */
        c[i] = (b[i] << 2) + i;
        
        /* Another true dependence chain */
        d[i] = d[i-1] + a[i] * 3;
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, 
                         int *restrict z, int n) {
    int i;
    
    /* Different loop with register-to-register dependencies */
    int acc = x[0];
    for (i = 1; i < n; ++i) {
        /* Multiple true dependencies in same statement */
        acc = acc * y[i] + z[i];
        
        /* Write to array creates anti-dependence for next iteration */
        x[i] = acc + i;
        
        /* Cross-iteration dependencies between different arrays */
        y[i] = x[i-1] & 0xFF;
        
        /* Simple arithmetic with carried dependency */
        z[i] = z[i-1] + (x[i] >> 1);
    }
    
    return acc + x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int N = GLOBAL_N;  /* Use volatile variable to prevent constant folding */
    int result1, result2;
    
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
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (int i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 29) % 983;
        
        x[i] = (i * 41) % 977;
        y[i] = (i * 43) % 971;
        z[i] = (i * 47) % 967;
    }
    
    /* Call loops with dependencies to trigger DDG construction */
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
