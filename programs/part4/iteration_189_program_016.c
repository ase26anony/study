/* gcov-vector-comparisons.c
 * Designed to trigger vectorization of GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * comparisons in GCC's tree-vect-stmts.cc (lines 12216-12233)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __GNUC__
#define OPTIMIZE __attribute__((optimize("O3", "tree-vectorize")))
#else
#define OPTIMIZE
#endif

#define N 1024  /* Multiple of typical vector widths (128, 256, 512 bits) */

/* Worker function containing vectorizable comparison loops */
OPTIMIZE
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
OPTIMIZE
static int process_fp_comparisons(const float* a, const float* b, int n) {
    char gt_results[N];
    char ge_results[N];
    char lt_results[N];
    char le_results[N];
    
    int i;
    
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
static void init_arrays(short* a, short* b, int n, int seed) {
    for (int i = 0; i < n; i++) {
        /* Use different but deterministic patterns */
        a[i] = (short)((i * 37 + seed) % 1000);
        b[i] = (short)((i * 73 + seed * 2) % 1000);
    }
}

static void init_fp_arrays(float* a, float* b, int n, int seed) {
    for (int i = 0; i < n; i++) {
        a[i] = (float)((i * 37 + seed) % 1000) / 10.0f;
        b[i] = (float)((i * 73 + seed * 2) % 1000) / 10.0f;
    }
}

int main(int argc, char** argv) {
    short a[N], b[N];
    float fa[N], fb[N];
    
    /* Use argv to get seed value to prevent constant propagation */
    int seed = (argc > 1) ? atoi(argv[1]) : 42;
    
    /* Initialize with non-constant data */
    init_arrays(a, b, N, seed);
    init_fp_arrays(fa, fb, N, seed + 1);
    
    /* Process multiple times with different data to ensure execution */
    int total_checksum = 0;
    
    /* Integer comparisons */
    total_checksum += process_comparisons(a, b, N);
    
    /* Swap arrays and process again */
    total_checksum += process_comparisons(b, a, N);
    
    /* Floating-point comparisons (requires -ffast-math for vectorization) */
    total_checksum += process_fp_comparisons(fa, fb, N);
    total_checksum += process_fp_comparisons(fb, fa, N);
    
    /* Use volatile to ensure results aren't optimized away */
    volatile int final_result = total_checksum;
    
    printf("Result: %d\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
