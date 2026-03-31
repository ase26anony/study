/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
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
        
        /* Anti-dependence: b[i] is written, but b[i] is read in next iteration */
        /* Also flow dependence: b[i] depends on a[i] from current iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more dependencies */
        /* Output dependence: c[i] is written twice in same iteration */
        c[i] = c[i] + (b[i] << 2);
        c[i] = c[i] * 3;
        
        /* Flow dependence: d[i] depends on b[i-1] from previous iteration */
        d[i] = d[i-1] + b[i-1];
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Separate initialization function */
__attribute__((noinline))
static void init_arrays(int *a, int *b, int *c, int *d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        /* Deterministic but non-constant initialization */
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
    
    /* Initialize arrays with pseudo-random values */
    init_arrays(a, b, c, d, N);
    
    /* Execute the loop with dependencies */
    result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Verify with simple computation */
    printf("Verification: a[%d]=%d, b[%d]=%d\n", 
           N-1, a[N-1], N-1, b[N-1]);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
