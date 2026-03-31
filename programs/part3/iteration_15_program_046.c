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
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, but b[i-1] was read in previous iteration
           if we had: c[i] = b[i-1] somewhere */
        
        /* Output dependence: multiple writes to same array element in same iteration */
        a[i] = a[i] + 1;  /* Second write to a[i] creates output dependence */
        
        /* Another true dependence: b[i] depends on a[i] computed above */
        b[i] = a[i] - d[i];
        
        /* Create anti-dependence by reading b[i-1] */
        c[i] = b[i-1] + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, 
                         int *restrict z, int n) {
    int i;
    
    /* Different pattern: chain of dependencies */
    for (i = 2; i < n; ++i) {
        /* Chain of true dependencies across iterations */
        x[i] = x[i-1] + x[i-2];
        
        /* Cross-iteration dependencies between different arrays */
        y[i] = x[i-1] * y[i-1];
        
        /* Multiple operations to increase DDG complexity */
        z[i] = (x[i] << 2) - y[i];
        z[i] = z[i] + (y[i-1] & 0xFF);  /* Output dependence on z[i] */
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i, result1, result2;
    int N = GLOBAL_N;  /* Use volatile variable to prevent constant folding */
    
    /* Allocate arrays with restrict to help alias analysis */
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
        y[i] = (i * 103) % 971;
        z[i] = (i * 107) % 967;
    }
    
    /* Call loops with dependencies */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
