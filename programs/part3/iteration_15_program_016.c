/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile for loop bound to prevent constant propagation */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple dependency types to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* TRUE DEPENDENCY (flow/RAW): a[i] depends on a[i-1] from previous iteration */
        /* This creates a carried true dependence with distance 1 */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* ANTI-DEPENDENCY (WAR): b[i] is written, then b[i-1] might be read later */
        /* This creates anti-dependence between iterations */
        b[i] = a[i] - d[i];
        
        /* OUTPUT DEPENDENCY (WAW): a[i] is written twice in same iteration */
        /* This creates output dependence within same iteration */
        a[i] = a[i] + (b[i] >> 2);  /* Additional operation on a[i] */
        
        /* Another true dependency chain with different distance */
        c[i] = c[i-1] + (d[i] << 1);
        
        /* Cross-iteration dependency with distance 2 */
        if (i >= 2) {
            d[i] = d[i-2] * 3 + a[i-1];
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper to initialize arrays with pseudo-random values */
static void init_arrays(int *a, int *b, int *c, int *d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        /* Deterministic but non-constant values */
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
    }
}

int main(void) {
    int result;
    
    /* Dynamically allocate arrays to avoid stack overflow */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant values */
    init_arrays(a, b, c, d, N);
    
    /* Call the loop function - this should trigger DDG construction */
    result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Verify computation with simple check */
    if (result != 0) {
        printf("Computation completed successfully\n");
    }
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
