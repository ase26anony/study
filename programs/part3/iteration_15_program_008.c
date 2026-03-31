/* test_ddg.c - Program to trigger DDG edge creation in GCC's modulo scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Volatile to prevent constant propagation */
volatile int N = 1024;

/* Noinline to ensure loop stays as separate function */
__attribute__((noinline))
int compute_loop(int* restrict a, int* restrict b, 
                 int* restrict c, int* restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies for DDG construction */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written here, read in next iteration's a[i] calculation */
        b[i] = a[i] - d[i];
        
        /* Additional operation to create more edges in DDG */
        c[i] = (b[i-1] << 2) + i;  /* Anti-dependence on b[i-1] */
        
        /* Output dependence: d is written twice in same iteration */
        d[i] = a[i] * 3;
        d[i] = d[i] + b[i];  /* Second write creates output dependence */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

int main(void) {
    int i;
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
    
    /* Initialize with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
    }
    
    /* Call the loop function - this should trigger DDG construction */
    result = compute_loop(a, b, c, d, N);
    
    /* Print result to prevent optimization */
    printf("Checksum result: %d\n", result);
    
    /* Verify with simple calculation */
    int verify = 0;
    for (i = 0; i < N; ++i) {
        verify += a[i] + b[i] + c[i] + d[i];
    }
    printf("Total sum: %d\n", verify);
    
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
