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
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operation to create more edges in the DDG */
        c[i] = (b[i-1] << 2) + i;  /* Anti-dependence on b[i-1] */
        
        /* Output dependence: d is written multiple times in same iteration */
        d[i] = d[i] * 3 + 1;
        d[i] = d[i] & 0xFF;  /* Second write to d[i] creates output dependence */
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Helper function to initialize arrays */
void init_arrays(int* a, int* b, int* c, int* d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        /* Deterministic but non-trivial initialization */
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1001;
        c[i] = (i * 71) % 1001;
        d[i] = (i * 97) % 1001;
    }
}

int main(void) {
    int result;
    
    /* Allocate arrays with restrict to help alias analysis */
    int* restrict a = (int*)malloc(N * sizeof(int));
    int* restrict b = (int*)malloc(N * sizeof(int));
    int* restrict c = (int*)malloc(N * sizeof(int));
    int* restrict d = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    init_arrays(a, b, c, d, N);
    
    /* Execute the loop with dependencies */
    result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent optimization */
    printf("Checksum result: %d\n", result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
