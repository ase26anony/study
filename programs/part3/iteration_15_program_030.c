/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int* restrict a, int* restrict b, int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create a non-trivial DDG */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written here, but b[i-1] was read in previous iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more dependencies */
        c[i] = b[i-1] + a[i];  /* Anti-dependence on b[i-1] */
        d[i] = c[i] * 2 - d[i-1];  /* Flow dependence on d[i-1] */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with output dependencies */
__attribute__((noinline))
int compute_loop2(int* restrict x, int* restrict y, int n) {
    int i;
    
    for (i = 0; i < n; ++i) {
        /* Output dependence: x[i] written twice in same iteration */
        x[i] = y[i] * 3;
        x[i] = x[i] + (i & 0xFF);  /* Second write to x[i] */
        
        /* Flow dependence with distance > 1 */
        if (i >= 2) {
            y[i] = x[i-2] << 1;  /* Distance-2 flow dependence */
        }
    }
    
    return x[n-1] + y[n-1];
}

int main(void) {
    int i;
    int result1, result2;
    
    /* Dynamically allocate arrays to avoid stack overflow */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    int* d = (int*)malloc(N * sizeof(int));
    int* x = (int*)malloc(N * sizeof(int));
    int* y = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
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
    }
    
    /* Call the loop functions - these should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, N);
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
