/* Vector comparison test to cover GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector width (128/256/512 bits) */

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
    
    /* Loop 1: GT_EXPR (>) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
    for (i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]);
    }
    
    /* Loop 2: GE_EXPR (>=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
    for (i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]);
    }
    
    /* Loop 3: LT_EXPR (<) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
    for (i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]);
    }
    
    /* Loop 4: LE_EXPR (<=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
    for (i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]);
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Alternative version with float comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_float_comparisons(const float* a, const float* b, int n) {
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* Separate loops for each comparison type */
    for (i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]);
    }
    
    for (i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]);
    }
    
    for (i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]);
    }
    
    for (i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]);
    }
    
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant patterns */
static void init_arrays(short* a, short* b, float* fa, float* fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use different patterns to avoid constant propagation */
        a[i] = (short)((i * 1789 + seed) % 32767);
        b[i] = (short)((i * 1787 + seed * 3) % 32767);
        fa[i] = (float)((i * 1789 + seed) % 1000) / 100.0f;
        fb[i] = (float)((i * 1787 + seed * 3) % 1000) / 100.0f;
    }
}

int main(int argc, char* argv[]) {
    short array_a[N], array_b[N];
    float farray_a[N], farray_b[N];
    
    /* Use argv for seed to prevent compile-time evaluation */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize with non-constant data */
    init_arrays(array_a, array_b, farray_a, farray_b, seed);
    
    /* Process comparisons multiple times with different data */
    volatile int total_checksum = 0;
    
    /* Call integer version */
    total_checksum += process_comparisons(array_a, array_b, N);
    
    /* Modify arrays slightly */
    for (int i = 0; i < N; i++) {
        array_a[i] += (short)(i % 100);
        array_b[i] -= (short)(i % 50);
    }
    
    /* Call again with modified data */
    total_checksum += process_comparisons(array_a, array_b, N);
    
    /* Call float version */
    total_checksum += process_float_comparisons(farray_a, farray_b, N);
    
    /* Modify float arrays */
    for (int i = 0; i < N; i++) {
        farray_a[i] += (float)(i % 10) * 0.1f;
        farray_b[i] -= (float)(i % 5) * 0.1f;
    }
    
    /* Call float version again */
    total_checksum += process_float_comparisons(farray_a, farray_b, N);
    
    /* Use the result to prevent optimization */
    printf("Total checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
