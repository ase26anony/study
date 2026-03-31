/* Program to trigger vector comparison lowering in GCC's tree-vect-stmts.cc
 * Specifically targets GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering to bitwise ops
 */

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
    char gt_res[N];
    char ge_res[N];
    char lt_res[N];
    char le_res[N];
    
    int i;
    
    /* Loop 1: GT_EXPR (>) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
    for (i = 0; i < n; i++) {
        gt_res[i] = (a[i] > b[i]);
    }
    
    /* Loop 2: GE_EXPR (>=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
    for (i = 0; i < n; i++) {
        ge_res[i] = (a[i] >= b[i]);
    }
    
    /* Loop 3: LT_EXPR (<) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
    for (i = 0; i < n; i++) {
        lt_res[i] = (a[i] < b[i]);
    }
    
    /* Loop 4: LE_EXPR (<=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
    for (i = 0; i < n; i++) {
        le_res[i] = (a[i] <= b[i]);
    }
    
    /* Combine results to prevent dead code elimination */
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_res[i] + ge_res[i] + lt_res[i] + le_res[i];
    }
    
    return checksum;
}

/* Alternative with floating-point comparisons */
#ifdef __GNUC__
__attribute__((optimize("O3", "tree-vectorize", "fast-math")))
#endif
static int process_fp_comparisons(const float* fa, const float* fb, int n) {
    char gt_fp[N];
    char ge_fp[N];
    char lt_fp[N];
    char le_fp[N];
    
    int i;
    
    /* Floating-point GT */
    for (i = 0; i < n; i++) {
        gt_fp[i] = (fa[i] > fb[i]);
    }
    
    /* Floating-point GE */
    for (i = 0; i < n; i++) {
        ge_fp[i] = (fa[i] >= fb[i]);
    }
    
    /* Floating-point LT */
    for (i = 0; i < n; i++) {
        lt_fp[i] = (fa[i] < fb[i]);
    }
    
    /* Floating-point LE */
    for (i = 0; i < n; i++) {
        le_fp[i] = (fa[i] <= fb[i]);
    }
    
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_fp[i] + ge_fp[i] + lt_fp[i] + le_fp[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant patterns */
static void init_arrays(short* a, short* b, float* fa, float* fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use seed to create varying but deterministic patterns */
        a[i] = (short)((i * 1789 + seed * 7919) % 32767);
        b[i] = (short)((i * 1789 + seed * 7919 + 12345) % 32767);
        
        /* For floating-point, create values with clear ordering relationships */
        fa[i] = (float)(i * 1.5f + seed * 0.1f);
        fb[i] = (float)(i * 1.5f + seed * 0.1f + 0.3f);
    }
}

int main(int argc, char** argv) {
    short array_a[N];
    short array_b[N];
    float farray_a[N];
    float farray_b[N];
    
    /* Use argv to create varying seeds */
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
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
    
    /* Additional test with different data pattern */
    init_arrays(array_a, array_b, farray_a, farray_b, seed + 1);
    int_result += process_comparisons(array_a, array_b, N);
    fp_result += process_fp_comparisons(farray_a, farray_b, N);
    
    printf("After second run - Total: %d\n", int_result + fp_result);
    
    return (int_result + fp_result) > 0 ? 0 : 1;
}
