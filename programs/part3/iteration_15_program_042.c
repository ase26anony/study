/* test_modulo_sched.c
 * Program designed to trigger DDG edge creation in GCC's modulo scheduler
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
    
    /* Loop with multiple carried dependencies to create DDG edges */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, then potentially read in next iteration */
        /* Also creates output dependence on b[i] within same iteration */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more edges */
        c[i] = (b[i] << 2) + i;      /* Creates anti-dependence on b[i] */
        d[i] = a[i-1] + b[i-1];      /* True dependence across iterations */
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different pattern to increase chances */
__attribute__((noinline))
int compute_loop2(int *restrict x, int *restrict y, 
                  int *restrict z, int n) {
    int i;
    
    /* Different dependency pattern */
    for (i = 2; i < n; ++i) {
        /* Multiple true dependencies */
        x[i] = x[i-1] + x[i-2];
        
        /* Anti-dependence chain */
        y[i] = x[i] * y[i-1];
        
        /* Output dependence within iteration */
        z[i] = y[i] + i;
        z[i] = z[i] * 3;  /* Second write to z[i] creates output dependence */
    }
    
    return x[n-1] + y[n-1] + z[n-1];
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
    int *restrict z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        
        x[i] = (i * 101) % 977;
        y[i] = (i * 103) % 971;
        z[i] = (i * 107) % 967;
    }
    
    /* Execute loops with carried dependencies */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Print results to prevent dead code elimination */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
