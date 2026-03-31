/* test_ddg_edge.c
 * Designed to trigger GCC's DDG edge creation in modulo scheduling.
 * Compile with: gcc -O2 -fmodulo-sched -fdump-rtl-sms test_ddg_edge.c -o test_ddg_edge
 */

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
        
        /* Anti-dependence: b[i] is written, then b[i-1] might be read in next iteration
           if we had: c[i] = b[i-1] + ... */
        b[i] = a[i] - d[i];
        
        /* Output dependence within same iteration (less common but possible) */
        /* Uncomment to add output dependence:
        int temp = a[i] >> 2;
        a[i] = temp + 1;
        */
        
        /* Additional operation to create more edges */
        c[i] = (b[i] << 1) + i;
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1];
}

/* Another loop variant with different dependency patterns */
__attribute__((noinline))
static int compute_loop2(int *restrict x, int *restrict y, 
                         int *restrict z, int n) {
    int i;
    
    /* Different dependency pattern: chain of dependencies */
    for (i = 2; i < n; ++i) {
        /* Multiple true dependencies forming a chain */
        x[i] = x[i-1] + y[i-2];
        y[i] = x[i] * z[i];
        z[i] = y[i] - x[i-1];
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i;
    int result1, result2;
    
    /* Dynamically allocate arrays to avoid stack overflow for large N */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    int *x = (int*)malloc(N * sizeof(int));
    int *y = (int*)malloc(N * sizeof(int));
    int *z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values
       This prevents the compiler from optimizing everything away */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1001;
        c[i] = (i * 71) % 1001;
        d[i] = (i * 89) % 1001;
        x[i] = (i * 101) % 1001;
        y[i] = (i * 131) % 1001;
        z[i] = (i * 151) % 1001;
    }
    
    /* Call the loop functions - these should trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d\n", result1, result2);
    
    /* Additional computation to potentially create more optimization opportunities */
    int sum = 0;
    for (i = 0; i < 100; ++i) {
        sum += a[i] + b[i];
    }
    printf("Checksum: %d\n", sum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
