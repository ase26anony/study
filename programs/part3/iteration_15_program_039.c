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
    
    /* Loop with multiple types of dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* TRUE (FLOW) DEPENDENCE: a[i] depends on a[i-1] (read-after-write across iterations) */
        int temp = a[i-1] * b[i];
        
        /* More operations to create a non-trivial DDG */
        temp = temp + c[i];
        
        /* OUTPUT DEPENDENCE: a[i] is written multiple times in same iteration */
        a[i] = temp;
        a[i] = a[i] + (i & 0x1F);  /* Additional write to create output dependence */
        
        /* ANTI-DEPENDENCE: b[i] is written, then b[i-1] might be read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operation with flow dependence on b[i] */
        c[i] = b[i] << 2;
        
        /* Another flow dependence chain */
        d[i] = d[i-1] + (a[i] >> 3);
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper function to initialize arrays with deterministic values */
static void init_arrays(int *a, int *b, int *c, int *d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        /* Deterministic but non-trivial initialization */
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
    }
}

int main(void) {
    int result;
    
    /* Use local variable to avoid constant folding of N */
    int n = N;
    
    /* Allocate arrays with restrict to help alias analysis */
    int *restrict a = (int*)malloc(n * sizeof(int));
    int *restrict b = (int*)malloc(n * sizeof(int));
    int *restrict c = (int*)malloc(n * sizeof(int));
    int *restrict d = (int*)malloc(n * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, c, d, n);
    
    /* Execute the loop with dependencies */
    result = compute_loop(a, b, c, d, n);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Free allocated memory */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
