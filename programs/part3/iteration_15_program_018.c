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
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more DDG edges */
        c[i] = b[i-1] + i;      /* Anti-dependence on b[i-1] */
        d[i] = d[i-1] << 2;     /* True dependence on d[i-1] */
        
        /* Output dependence: multiple writes to same array in same iteration */
        a[i] = a[i] + (b[i] >> 1);
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper to initialize arrays with deterministic values */
static void init_arrays(int *a, int *b, int *c, int *d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
    }
}

int main(void) {
    int *a, *b, *c, *d;
    int result;
    
    /* Dynamically allocate arrays to avoid stack overflow */
    a = (int*)malloc(N * sizeof(int));
    b = (int*)malloc(N * sizeof(int));
    c = (int*)malloc(N * sizeof(int));
    d = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with deterministic values */
    init_arrays(a, b, c, d, N);
    
    /* Execute the loop with carried dependencies */
    result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum result: %d\n", result);
    
    /* Verify with a simple calculation */
    printf("Expected (approx): %ld\n", 
           (long)((N-1) * 37 % 1001) + ((N-1) * 53 % 997) + 
           ((N-1) * 71 % 991) + ((N-1) * 89 % 983));
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
