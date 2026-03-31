/* Test program to cover vector comparison lowering in tree-vect-stmts.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector width */

/* Worker function with vectorizable comparison loops */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short* a, const short* b, int n) {
    /* Destination arrays for comparison results */
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* Loop 1: GT_EXPR (>) */
    for (i = 0; i < n; i++) {
        gt_results[i] = a[i] > b[i];
    }
    
    /* Loop 2: GE_EXPR (>=) */
    for (i = 0; i < n; i++) {
        ge_results[i] = a[i] >= b[i];
    }
    
    /* Loop 3: LT_EXPR (<) */
    for (i = 0; i < n; i++) {
        lt_results[i] = a[i] < b[i];
    }
    
    /* Loop 4: LE_EXPR (<=) */
    for (i = 0; i < n; i++) {
        le_results[i] = a[i] <= b[i];
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant patterns */
static void init_arrays(short* a, short* b, int n, int seed) {
    for (int i = 0; i < n; i++) {
        /* Use different patterns for a and b to ensure comparisons vary */
        a[i] = (short)((i * 1789 + seed * 7919) % 32767);
        b[i] = (short)((i * 2017 + seed * 6827) % 32767);
        
        /* Ensure some elements are equal to test equality cases */
        if (i % 7 == 0) {
            b[i] = a[i];
        }
    }
}

int main(int argc, char** argv) {
    short array_a[N];
    short array_b[N];
    
    /* Use argv to get seed value to prevent compile-time evaluation */
    int seed = 42;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with different seeds to vary patterns */
    init_arrays(array_a, array_b, N, seed);
    
    /* Process multiple times to ensure execution */
    int total = 0;
    for (int iter = 0; iter < 3; iter++) {
        total += process_comparisons(array_a, array_b, N);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < N; i++) {
            array_a[i] += (short)(iter * 17);
            array_b[i] += (short)(iter * 23);
        }
    }
    
    /* Use result to prevent optimization */
    printf("Result checksum: %d\n", total);
    
    return 0;
}
