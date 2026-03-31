/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Use volatile for loop bound to prevent constant propagation */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
static int compute_loop(int* restrict a, int* restrict b, 
                       int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written here, read in next iteration's a[i] calculation */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more DDG edges */
        /* Output dependence: c[i] written twice in same iteration */
        c[i] = d[i] << 1;
        c[i] = c[i] + i;  /* Second write to c[i] creates output dependence */
        
        /* Another true dependence chain */
        d[i] = d[i-1] + (b[i] >> 2);
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Separate initialization function to avoid mixing with main loop */
__attribute__((noinline))
static void init_arrays(int* a, int* b, int* c, int* d, int n) {
    int i;
    for (i = 0; i < n; ++i) {
        /* Deterministic but non-constant initialization */
        a[i] = (i * 37) % 1001;
        b[i] = (i * 51) % 997;
        c[i] = (i * 73) % 991;
        d[i] = (i * 19) % 983;
    }
}

int main(void) {
    int result;
    
    /* Dynamically allocate arrays to avoid stack overflow */
    int* a = (int*)malloc(N * sizeof(int));
    int* b = (int*)malloc(N * sizeof(int));
    int* c = (int*)malloc(N * sizeof(int));
    int* d = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with non-constant values */
    init_arrays(a, b, c, d, N);
    
    /* Execute the loop with dependencies */
    result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent dead code elimination */
    printf("Checksum result: %d\n", result);
    
    /* Verify with simple calculation */
    printf("Expected (approximate): %d\n", 
           ((N-1)*37%1001 + (N-1)*51%997 + (N-1)*73%991 + (N-1)*19%983));
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
