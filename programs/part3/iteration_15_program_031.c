/* test_ddg.c - Program to trigger GCC's DDG edge creation for modulo scheduling */

#include <stdio.h>
#include <stdlib.h>

/* Volatile variable to prevent constant propagation */
volatile int N = 1024;

/* Function containing the loop with carried dependencies */
__attribute__((noinline))
int compute_loop(int *restrict a, int *restrict b, 
                 int *restrict c, int *restrict d, int n) {
    int i;
    
    /* Loop with multiple carried dependencies to create a non-trivial DDG */
    for (i = 1; i < n; ++i) {
        /* True (flow) dependence: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] * b[i] + c[i];
        
        /* Anti-dependence: b[i] is written, but b[i-1] could be read in next iteration
           if we had: c[i] = b[i-1] + ... (we'll add this below) */
        
        /* Output dependence: multiple writes to same array element in same iteration */
        a[i] = a[i] + (b[i] << 2);  // Additional write to a[i]
        
        /* Another true dependence: b[i] depends on a[i] computed above */
        b[i] = a[i] - d[i];
        
        /* Anti-dependence: reading b[i-1] after writing b[i] in previous iteration */
        c[i] = b[i-1] + d[i];
    }
    
    /* Simple checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1];
}

/* Another loop with different pattern to increase DDG complexity */
__attribute__((noinline))
int compute_loop2(int *restrict x, int *restrict y, 
                  int *restrict z, int n) {
    int i;
    
    /* Different dependency pattern */
    for (i = 2; i < n; ++i) {
        /* Multiple interleaved dependencies */
        x[i] = x[i-1] + y[i-2];
        y[i] = x[i] * z[i];
        z[i] = y[i-1] - x[i-1];
        
        /* Additional operations to create more edges */
        x[i] = x[i] | (y[i] & 0xFF);
        y[i] = y[i] ^ (z[i] << 1);
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i, result1, result2;
    
    /* Use volatile to prevent compile-time computation */
    volatile int size = N;
    int actual_size = size;
    
    /* Allocate arrays with restrict to help alias analysis */
    int *restrict a = (int*)malloc(actual_size * sizeof(int));
    int *restrict b = (int*)malloc(actual_size * sizeof(int));
    int *restrict c = (int*)malloc(actual_size * sizeof(int));
    int *restrict d = (int*)malloc(actual_size * sizeof(int));
    int *restrict x = (int*)malloc(actual_size * sizeof(int));
    int *restrict y = (int*)malloc(actual_size * sizeof(int));
    int *restrict z = (int*)malloc(actual_size * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < actual_size; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 997;
        c[i] = (i * 71) % 991;
        d[i] = (i * 89) % 983;
        x[i] = (i * 101) % 977;
        y[i] = (i * 113) % 971;
        z[i] = (i * 127) % 967;
    }
    
    /* Call the loop functions multiple times to give compiler more chances */
    result1 = compute_loop(a, b, c, d, actual_size);
    result2 = compute_loop2(x, y, z, actual_size);
    
    /* Use results to prevent dead code elimination */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Free memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
