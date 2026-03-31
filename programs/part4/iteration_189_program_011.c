/* Vector comparison test to cover GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector widths (128/256/512 bits) */

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

/* Alternative version with floating-point comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_fp_comparisons(const float* a, const float* b, int n) {
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* Separate loops to ensure each comparison type gets its own vectorization */
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

/* Initialize arrays with non-constant pattern */
static void init_arrays(short* a, short* b, float* fa, float* fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use seed to create varying but deterministic patterns */
        a[i] = (short)((i * 1789 + seed * 37) % 32767);
        b[i] = (short)((i * 2017 + seed * 41) % 32767);
        fa[i] = (float)((i * 1789 + seed * 37) % 1000) / 100.0f;
        fb[i] = (float)((i * 2017 + seed * 41) % 1000) / 100.0f;
    }
}

int main(int argc, char* argv[]) {
    short array_a[N];
    short array_b[N];
    float farray_a[N];
    float farray_b[N];
    
    /* Use argv for seed variation to prevent constant propagation */
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with different seeds to create varying comparison patterns */
    init_arrays(array_a, array_b, farray_a, farray_b, seed);
    
    /* Process integer comparisons */
    int int_result = process_comparisons(array_a, array_b, N);
    
    /* Process floating-point comparisons (with fast-math enabled) */
    int fp_result = process_fp_comparisons(farray_a, farray_b, N);
    
    /* Use results to prevent optimization */
    volatile int final_result = int_result + fp_result;
    
    printf("Comparison checksum: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
