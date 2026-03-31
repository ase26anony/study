/* Test program to cover vector comparison lowering in tree-vect-stmts.cc
 * Lines 12216-12233: GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR lowering to bitwise ops
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

/* Additional test with floating point using -ffast-math */
#ifdef __GNUC__
__attribute__((optimize("O3", "fast-math", "tree-vectorize")))
#endif
static int process_float_comparisons(const float* a, const float* b, int n) {
    char gt_res[N];
    char ge_res[N];
    char lt_res[N];
    char le_res[N];
    
    int i;
    
    /* Separate loops for each comparison type */
    for (i = 0; i < n; i++) gt_res[i] = (a[i] > b[i]);
    for (i = 0; i < n; i++) ge_res[i] = (a[i] >= b[i]);
    for (i = 0; i < n; i++) lt_res[i] = (a[i] < b[i]);
    for (i = 0; i < n; i++) le_res[i] = (a[i] <= b[i]);
    
    int checksum = 0;
    for (i = 0; i < n; i++) {
        checksum += gt_res[i] + ge_res[i] + lt_res[i] + le_res[i];
    }
    
    return checksum;
}

/* Initialize arrays with non-constant pattern */
static void init_arrays(short* a, short* b, float* fa, float* fb, int seed) {
    for (int i = 0; i < N; i++) {
        /* Use different but deterministic patterns */
        a[i] = (short)((i * 37 + seed) % 1000);
        b[i] = (short)((i * 73 + seed * 2) % 1000);
        fa[i] = (float)((i * 37 + seed) % 1000) / 10.0f;
        fb[i] = (float)((i * 73 + seed * 2) % 1000) / 10.0f;
    }
}

int main(int argc, char** argv) {
    short array_a[N], array_b[N];
    float farray_a[N], farray_b[N];
    
    /* Use argv to get seed value to prevent constant propagation */
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Initialize with non-constant data */
    init_arrays(array_a, array_b, farray_a, farray_b, seed);
    
    /* Process comparisons multiple times with different data */
    int total_checksum = 0;
    
    /* Test with integer (short) comparisons */
    total_checksum += process_comparisons(array_a, array_b, N);
    
    /* Modify arrays slightly and test again */
    for (int i = 0; i < N; i++) {
        array_a[i] += (short)(i % 10);
    }
    total_checksum += process_comparisons(array_a, array_b, N);
    
    /* Test with floating point comparisons */
    total_checksum += process_float_comparisons(farray_a, farray_b, N);
    
    /* Modify and test again */
    for (int i = 0; i < N; i++) {
        farray_a[i] += (float)(i % 5);
    }
    total_checksum += process_float_comparisons(farray_a, farray_b, N);
    
    /* Use the result to prevent dead code elimination */
    printf("Total checksum: %d\n", total_checksum);
    
    return total_checksum != 0 ? 0 : 1;
}
