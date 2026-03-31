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
        
        /* Anti-dependence: b[i] is written here, but b[i-1] was read in previous iteration */
        /* Also flow dependence: uses a[i] computed above */
        b[i] = a[i] - d[i];
        
        /* Additional operations to create more dependencies */
        /* Output dependence: c[i] is written multiple times in same iteration */
        c[i] = (c[i] << 1) | (c[i] >> 31);  /* Rotate left by 1 */
        c[i] = c[i] ^ (i & 0xFF);           /* XOR with iteration counter */
        
        /* Another flow dependence chain with d */
        d[i] = d[i-1] + (b[i] & 0x3F);
    }
    
    /* Compute checksum to prevent dead code elimination */
    return a[n-1] + b[n-1] + c[n-1] + d[n-1];
}

/* Another loop with different dependency patterns */
__attribute__((noinline))
int compute_loop2(int* restrict x, int* restrict y, 
                  int* restrict z, int n) {
    int i;
    
    /* Different loop with register pressure */
    for (i = 2; i < n; ++i) {
        /* Multiple interleaved dependencies */
        x[i] = x[i-1] + x[i-2];      /* Fibonacci-like flow dependence */
        y[i] = y[i-1] - x[i];        /* Flow + anti dependence */
        z[i] = z[i-1] * y[i] + i;    /* Flow dependence chain */
        
        /* Create loop-carried output dependence */
        x[i-1] = y[i] ^ z[i];        /* Modify previously computed value */
    }
    
    return x[n-1] + y[n-1] + z[n-1];
}

int main(void) {
    int i;
    int result1, result2;
    
    /* Allocate arrays with restrict to help alias analysis */
    int* restrict a = (int*)malloc(N * sizeof(int));
    int* restrict b = (int*)malloc(N * sizeof(int));
    int* restrict c = (int*)malloc(N * sizeof(int));
    int* restrict d = (int*)malloc(N * sizeof(int));
    
    int* restrict x = (int*)malloc(N * sizeof(int));
    int* restrict y = (int*)malloc(N * sizeof(int));
    int* restrict z = (int*)malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y || !z) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random but deterministic values */
    for (i = 0; i < N; ++i) {
        a[i] = (i * 37) % 1001;
        b[i] = (i * 53) % 1001;
        c[i] = (i * 71) % 1001;
        d[i] = (i * 97) % 1001;
        
        x[i] = (i * 131) % 1001;
        y[i] = (i * 151) % 1001;
        z[i] = (i * 173) % 1001;
    }
    
    /* Call loops with dependencies to trigger DDG construction */
    result1 = compute_loop(a, b, c, d, N);
    result2 = compute_loop2(x, y, z, N);
    
    /* Use results to prevent dead code elimination */
    printf("Checksum 1: %d\n", result1);
    printf("Checksum 2: %d\n", result2);
    printf("Total: %d\n", result1 + result2);
    
    /* Free allocated memory */
    free(a); free(b); free(c); free(d);
    free(x); free(y); free(z);
    
    return 0;
}
