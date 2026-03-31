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
    
    /* Loop with multiple carried dependencies for DDG construction */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, but b[i-1] might be read in next iteration
           if we had b[i-1] usage. We'll create anti-dependence differently. */
        
        /* Create anti-dependence by using b[i-1] in calculation */
        c[i] = b[i-1] + d[i];
        
        /* Output dependence: multiple writes to same array in same iteration */
        a[i] = a[i] + d[i];  /* Second write to a[i] creates output dependence */
        
        /* Another true dependence chain */
        b[i] = a[i] - d[i];
    }
    
    /* Return checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
int compute_loop2(int* restrict x, int* restrict y, 
                  int* restrict z, int n) {
    int i;
    
    /* Different loop with register-to-register dependencies */
    int acc = x[0];
    for (i = 1; i < n; ++i) {
        /* Multiple true dependencies creating a critical path */
        int temp1 = acc * y[i];
        int temp2 = temp1 + z[i];
        int temp3 = temp2 << 2;
        acc = temp3 - x[i];
        
        /* Store results creating memory dependencies */
        x[i] = acc;
        y[i] = temp2;
        
        /* Anti-dependence: reading y[i] after writing it in previous iteration */
        z[i] = y[i-1] + i;
    }
    
    return acc + x[n-1] + y[n-1];
}

int main(void) {
    int i;
    
    /* Allocate and initialize arrays with deterministic values */
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
    
    /* Initialize with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1001;
        c[i] = (i * 71) % 1001;
        d[i] = (i * 97) % 1001;
        
        x[i] = (i * 101) % 1001;
        y[i] = (i * 103) % 1001;
        z[i] = (i * 107) % 1001;
    }
    
    /* Call loops to force DDG construction */
    int result1 = compute_loop(a, b, c, d, N);
    int result2 = compute_loop2(x, y, z, N);
    
    /* Use results to prevent optimization */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Clean up */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
