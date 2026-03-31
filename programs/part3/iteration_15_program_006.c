/* test_ddg.c - Program to trigger GCC's Data Dependency Graph construction */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] (read-after-write) */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        /* Also creates output dependence on b[i] from previous statement */
        b[i] = a[i] - d[i];
        
        /* Additional operation to create more dependencies */
        c[i] = (b[i] << 2) + a[i-1];  /* Flow dependence on a[i-1], anti on b[i] */
        
        /* Output dependence: d is written twice in same iteration */
        d[i] = c[i] * 3;
        d[i] = d[i] + 1;  /* Output dependence on d[i] */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, 
                         int *restrict z, int n) {
    int i;
    
    /* Different dependency pattern to exercise more edge creation */
    for (i = 2; i < n; ++i) {
        /* Complex carried dependencies */
        x[i] = x[i-1] + x[i-2];  /* Flow dependence on two previous iterations */
        y[i] = y[i-1] * x[i];    /* Flow on y[i-1], flow on x[i] */
        z[i] = z[i-1] - y[i];    /* Flow on z[i-1], anti on y[i] */
        
        /* Create anti-dependence chain */
        y[i-1] = z[i] + 5;       /* Anti on z[i] from previous statement */
    }
    
    return x[n-1] + y[n-2] + z[n-1];
}

int main(void) {
    int i, result1, result2;
    
    /* Dynamically allocate arrays to avoid stack overflow with large N */
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
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1001;
        c[i] = (i * 71) % 1001;
        d[i] = (i * 97) % 1001;
        x[i] = (i * 113) % 1001;
        y[i] = (i * 131) % 1001;
        z[i] = (i * 151) % 1001;
    }
    
    /* Call the loop functions - force execution to prevent optimization */
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
