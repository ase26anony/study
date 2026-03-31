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
    
    /* Loop with multiple carried dependencies to create a complex DDG */
    for (i = 1; i < n; ++i) {
        /* TRUE (FLOW) DEPENDENCE: a[i] depends on a[i-1] (read-after-write across iterations) */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* ANTI-DEPENDENCE: b[i] is written, then read in next iteration as b[i-1] */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more edges in the DDG */
        /* Output dependence within same iteration (write-after-write) */
        c[i] = (a[i] << 2) | (b[i] & 0xF);
        
        /* Another flow dependence chain */
        d[i] = d[i-1] + (a[i] * 3);
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop variant with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, 
                         int *restrict z, int n) {
    int i;
    
    /* Different dependency pattern to exercise more edge creation */
    for (i = 2; i < n; ++i) {
        /* Multiple flow dependencies */
        x[i] = x[i-1] + x[i-2];
        
        /* Anti-dependence chain */
        y[i] = z[i-1] * 7;
        z[i] = y[i] - x[i];
        
        /* Memory dependencies with different distances */
        if (i > 10) {
            x[i] += z[i-10];  /* Longer distance dependency */
        }
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i, result1, result2;
    
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
    
    /* Call the loop functions - these should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
