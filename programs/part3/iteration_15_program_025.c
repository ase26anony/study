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
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        /* Also creates output dependence on b[i] itself */
        b[i] = a[i] - d[i];
        
        /* Additional operation to create more edges in the DDG */
        c[i] = (b[i-1] << 2) + i;  /* Anti-dependence on b[i-1] */
        
        /* Output dependence: d is written twice in same iteration */
        d[i] = d[i] * 3;
        d[i] = d[i] + 7;  /* Second write creates output dependence */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, int *c, int *d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 73) % 997;
        c[i] = (i * 13) % 991;
        d[i] = (i * 29) % 983;
    }
}

int main(void) {
    int n = GLOBAL_N;  /* Use volatile variable to prevent constant folding */
    int *a, *b, *c, *d;
    int result;
    
    /* Allocate arrays with restrict qualifiers to help alias analysis */
    a = (int*)malloc(n * sizeof(int));
    b = (int*)malloc(n * sizeof(int));
    c = (int*)malloc(n * sizeof(int));
    d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, c, d, n);
    
    /* Execute the loop with dependencies */
    result = compute_loop(a, b, c, d, n);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum result: %d\n", result);
    
    /* Verify with a simple calculation */
    {
        int expected = 0;
        int i;
        /* Recompute in a different way for verification */
        for (i = 0; i < n; ++i) {
            expected += a[i] + b[i] + c[i] + d[i];
        }
        printf("Total sum: %d\n", expected);
    }
    
    /* Clean up */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
