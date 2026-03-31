/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent constant propagation */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple dependency types to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, but b[i-1] might be read in next iteration
           if we had: c[i] = b[i-1] + ... (we'll add this below) */
        
        /* Output dependence: a[i] is written twice in same iteration */
        a[i] = a[i] + (b[i] << 2);  /* Additional write to a[i] */
        
        /* Another true dependence: b[i] depends on a[i] computed above */
        b[i] = a[i] - d[i];
        
        /* Anti-dependence: reading b[i-1] after it was written in previous iteration */
        c[i] = b[i-1] + d[i];
    }
    
    /* Return checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1];
}

/* Separate initialization function to avoid polluting the main loop */
__attribute__((noinline))
static void init_arrays(int *restrict a, int *restrict b, 
                        int *restrict c, int *restrict d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        /* Deterministic but non-trivial initialization */
        a[i] = (i * 37) % 1001;
        b[i] = (i * 51) % 997;
        c[i] = (i * 73) % 991;
        d[i] = (i * 29) % 983;
    }
}

int main(void) {
    int result;
    
    /* Use volatile N to prevent compile-time optimization */
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
    printf("Checksum result: %d\n", result);
    
    /* Verify with a simple computation */
    {
        int verify = 0;
        int i;
        for (i = 0; i < n; ++i) {
            verify += a[i] + b[i] + c[i] + d[i];
        }
        printf("Total sum: %d\n", verify);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
