/* Vectorizable comparison test targeting tree-vect-stmts.cc lines 12216-12233 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024  /* Multiple of typical vector width (128/256/512 bits) */

/* Worker function with vectorizable comparison loops */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize")))
#endif
static int process_comparisons(const short *a, const short *b, int n) {
    /* Destination arrays for comparison results */
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* Loop 1: GT_EXPR (>) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
    for (i = 0; i < n; i++) {
        gt_results[i] = (char)(a[i] > b[i]);
    }
    
    /* Loop 2: GE_EXPR (>=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
    for (i = 0; i < n; i++) {
        ge_results[i] = (char)(a[i] >= b[i]);
    }
    
    /* Loop 3: LT_EXPR (<) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
    for (i = 0; i < n; i++) {
        lt_results[i] = (char)(a[i] < b[i]);
    }
    
    /* Loop 4: LE_EXPR (<=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
    for (i = 0; i < n; i++) {
        le_results[i] = (char)(a[i] <= b[i]);
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Alternative with floating-point comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_fp_comparisons(const float *fa, const float *fb, int n) {
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
    /* GT_EXPR with floats */
    for (i = 0; i < n; i++) {
        gt_results[i] = (char)(fa[i] > fb[i]);
    }
    
    /* GE_EXPR with floats */
    for (i = 0; i < n; i++) {
        ge_results[i] = (char)(fa[i] >= fb[i]);
    }
    
    /* LT_EXPR with floats */
    for (i = 0; i < n; i++) {
        lt_results[i] = (char)(fa[i] < fb[i]);
    }
    
    /* LE_EXPR with floats */
    for (i = 0; i < n; i++) {
        le_results[i] = (char)(fa[i] <= fb[i]);
    }
    
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_results[i] + ge_results[i] + lt_results[i] + le_results[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant pattern */
static void init_arrays(short *a, short *b, float *fa, float *fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use seed to create non-constant but predictable pattern */
        a[i] = (short)((i * 1789 + seed * 7919) % 32767);
        b[i] = (short)((i * 1787 + seed * 7927) % 32767);
        fa[i] = (float)((i * 1789 + seed * 7919) % 1000) / 100.0f;
        fb[i] = (float)((i * 1787 + seed * 7927) % 1000) / 100.0f;
    }
}

int main(int argc, char *argv[]) {
    short array_a[N], array_b[N];
    float farray_a[N], farray_b[N];
    
    /* Use argv for seed variation to prevent constant propagation */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    
    /* Initialize with non-constant data */
    init_arrays(array_a, array_b, farray_a, farray_b, seed);
    
    /* Process integer comparisons */
    int int_result = process_comparisons(array_a, array_b, N);
    
    /* Process floating-point comparisons */
    int fp_result = process_fp_comparisons(farray_a, farray_b, N);
    
    /* Use results to prevent optimization */
    printf("Integer comparison checksum: %d\n", int_result);
    printf("Float comparison checksum: %d\n", fp_result);
    printf("Total: %d\n", int_result + fp_result);
    
    return (int_result + fp_result) > 0 ? 0 : 1;
}
