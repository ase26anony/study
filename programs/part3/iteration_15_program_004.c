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
    
    /* Loop with multiple dependency types for DDG construction */
    for (i = 1; i < n; ++i) {
        /* TRUE (FLOW) DEPENDENCE: a[i] depends on a[i-1] (read-after-write across iterations) */
        /* This creates a carried true dependence with distance 1 */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* ANTI-DEPENDENCE: b[i] is written, then potentially read in next iteration */
        /* Also creates output dependence on a[i] from previous statement */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more DDG edges */
        /* Create anti-dependence: read c[i] before potentially writing in next iteration */
        c[i] = c[i-1] + (b[i] << 2);
        
        /* Output dependence on d[i] from initialization */
        d[i] = d[i-1] ^ (a[i] & 0xFF);
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper to initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, int *c, int *d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        /* Deterministic but non-trivial initialization */
        a[i] = (i * 37) % 1001;
        b[i] = (i * 73) % 1003;
        c[i] = (i * 13) % 997;
        d[i] = (i * 91) % 991;
    }
}

int main(void) {
    int result;
    
    /* Dynamically allocate arrays to avoid stack overflow for large N */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(a, b, c, d, N);
    
    /* Execute the loop with dependencies */
    result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum result: %d\n", result);
    
    /* Verify with a simple calculation */
    printf("Expected (for verification): %d\n", 
           ((N-1) * 37) % 1001 + ((N-1) * 73) % 1003 + 
           ((N-1) * 13) % 997 + ((N-1) * 91) % 991);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
