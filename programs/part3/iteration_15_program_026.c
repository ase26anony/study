/* test_modulo_sched.c
 * Designed to trigger DDG edge creation in GCC's modulo scheduler
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms test_modulo_sched.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Use volatile to prevent constant propagation and loop unrolling */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int *restrict a, int *restrict b, 
                 int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple dependency types to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] (read-after-write across iterations) */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more edges in the DDG */
        c[i] = (b[i-1] << 2) + i;  /* Anti-dependence on b[i-1] */
        d[i] = a[i] * 3 - b[i];    /* True dependence on a[i] and b[i] within same iteration */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with output dependencies */
__attribute__((noinline))
int compute_loop2(int *restrict x, int *restrict y, int n) {
    int i;
    
    /* Loop with output dependence (write-after-write) */
    for (i = 0; i < n; ++i) {
        x[i] = i * 2;      /* First write to x[i] */
        x[i] = x[i] + y[i]; /* Second write to x[i] - output dependence */
        y[i] = x[i] * 3;    /* True dependence on x[i] */
    }
    
    return x[n-1] + y[n-1];
}

int main(void) {
    int i, result1, result2;
    
    /* Allocate arrays with restrict to help alias analysis */
    int *restrict a = (int*)malloc(N * sizeof(int));
    int *restrict b = (int*)malloc(N * sizeof(int));
    int *restrict c = (int*)malloc(N * sizeof(int));
    int *restrict d = (int*)malloc(N * sizeof(int));
    
    int *restrict x = (int*)malloc(N * sizeof(int));
    int *restrict y = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values
     * This prevents the compiler from optimizing everything away */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1003;
        c[i] = (i * 71) % 1007;
        d[i] = (i * 97) % 1013;
        x[i] = i % 100;
        y[i] = (i * 3) % 200;
    }
    
    /* Call the loop functions - these should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, N);
    
    /* Use results to prevent dead code elimination */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y);
    
    return 0;
}
