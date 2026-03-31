/* Vector comparison test to cover GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector widths (128, 256, 512 bits) */

/* Worker function with explicit optimization attribute */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short* a, const short* b, int n) {
    /* Destination arrays for comparison results */
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    /* Separate loops for each comparison type to ensure distinct lowering */
    
    /* GT_EXPR: a[i] > b[i] */
    for (int i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]);
    }
    
    /* GE_EXPR: a[i] >= b[i] */
    for (int i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]);
    }
    
    /* LT_EXPR: a[i] < b[i] */
    for (int i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]);
    }
    
    /* LE_EXPR: a[i] <= b[i] */
    for (int i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]);
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum ^= (gt_results[i] + ge_results[i] * 3 + 
                     lt_results[i] * 5 + le_results[i] * 7);
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
    
    for (int i = 0; i < n; i++) {
        gt_results[i] = (a[i] > b[i]);
    }
    
    for (int i = 0; i < n; i++) {
        ge_results[i] = (a[i] >= b[i]);
    }
    
    for (int i = 0; i < n; i++) {
        lt_results[i] = (a[i] < b[i]);
    }
    
    for (int i = 0; i < n; i++) {
        le_results[i] = (a[i] <= b[i]);
    }
    
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum ^= (gt_results[i] + ge_results[i] * 3 + 
                     lt_results[i] * 5 + le_results[i] * 7);
    }
    
    return checksum;
}

/* Initialize arrays with non-constant patterns */
static void init_arrays(short* a, short* b, float* fa, float* fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use seed-dependent but non-constant values */
        a[i] = (short)((i * 1789 + seed * 7919) % 32767);
        b[i] = (short)((i * 2687 + seed * 9923) % 32767);
        fa[i] = (float)((i * 1789 + seed * 7919) % 1000) / 100.0f;
        fb[i] = (float)((i * 2687 + seed * 9923) % 1000) / 100.0f;
    }
}

int main(int argc, char** argv) {
    short array_a[N], array_b[N];
    float farray_a[N], farray_b[N];
    
    /* Use argv for seed variation to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize with non-constant data */
    init_arrays(array_a, array_b, farray_a, farray_b, seed);
    
    /* Process comparisons multiple times with different patterns */
    int total_checksum = 0;
    
    /* Integer comparisons */
    total_checksum ^= process_comparisons(array_a, array_b, N);
    
    /* Swap arrays and process again */
    total_checksum ^= process_comparisons(array_b, array_a, N);
    
    /* Floating-point comparisons (requires -ffast-math for vectorization) */
    total_checksum ^= process_fp_comparisons(farray_a, farray_b, N);
    total_checksum ^= process_fp_comparisons(farray_b, farray_a, N);
    
    /* Use volatile to ensure results aren't optimized away */
    volatile int final_result = total_checksum;
    
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
