/* test_vectorizable_condition.c
 * Designed to trigger specific uncovered lines in tree-vect-stmts.cc
 * Lines 12216-12233: GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR handling
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
__attribute__((noinline, pure))
int test_gt(int* restrict dst, const int* restrict src1, 
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2) {
    int sum = 0;
    #pragma GCC unroll 4
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* GE_EXPR (>=) */
__attribute__((noinline, pure))
int test_ge(int* restrict dst, const int* restrict src1,
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* LT_EXPR (<) */
__attribute__((noinline, pure))
int test_lt(int* restrict dst, const int* restrict src1,
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* LE_EXPR (<=) */
__attribute__((noinline, pure))
int test_le(int* restrict dst, const int* restrict src1,
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2) {
    int sum = 0;
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
int test_mixed_comparisons(int* restrict dst1, int* restrict dst2,
                          const int* restrict src1, const int* restrict src2,
                          const int* restrict val1, const int* restrict val2) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; ++i) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        sum += dst1[i];
        
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 4; ++j) {
            int idx = i * 4 + j;
            if (idx < N) {
                dst2[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
                sum += dst2[idx];
            }
        }
    }
    return sum;
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
int test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8],
                   const int val1[][8], const int val2[][8]) {
    int sum = 0;
    const int rows = N/8;
    
    /* First dimension uses GT_EXPR */
    for (int i = 0; i < rows; ++i) {
        /* Second dimension uses LE_EXPR */
        for (int j = 0; j < 8; ++j) {
            dst[i][j] = (i > j) ? 
                       ((src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j]) :
                       ((src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j]);
            sum += dst[i][j];
        }
    }
    return sum;
}

/* Complex pattern with multiple conditional assignments */
__attribute__((noinline))
int test_complex_pattern(int* restrict dst, const int* restrict src1,
                        const int* restrict src2, const int* restrict src3,
                        const int* restrict val1, const int* restrict val2) {
    int sum = 0;
    
    /* Mix of GE_EXPR and LT_EXPR in same loop */
    for (int i = 0; i < N; i += 2) {
        /* First comparison: GE_EXPR */
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
        
        /* Second comparison: LT_EXPR */
        if (i + 1 < N) {
            dst[i + 1] = (src3[i + 1] < src2[i + 1]) ? val1[i + 1] : val2[i + 1];
            sum += dst[i + 1];
        }
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], src3[N];
    ALIGNED int val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N];
    ALIGNED int dst_multi[N/8][8];
    ALIGNED int src1_multi[N/8][8], src2_multi[N/8][8];
    ALIGNED int val1_multi[N/8][8], val2_multi[N/8][8];
    
    int total_sum = 0;
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i * 3) % 17;
        src2[i] = (i * 5) % 13;
        src3[i] = (i * 7) % 19;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
    
    for (int i = 0; i < N/8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = i * 8 + j;
            src1_multi[i][j] = (idx * 3) % 17;
            src2_multi[i][j] = (idx * 5) % 13;
            val1_multi[i][j] = idx * 2;
            val2_multi[i][j] = idx * 3;
        }
    }
    
    /* Test each comparison operator */
    total_sum += test_gt(dst1, src1, src2, val1, val2);
    total_sum += test_ge(dst2, src1, src2, val1, val2);
    total_sum += test_lt(dst1, src1, src2, val1, val2);
    total_sum += test_le(dst2, src1, src2, val1, val2);
    
    /* Test mixed patterns */
    total_sum += test_mixed_comparisons(dst1, dst2, src1, src2, val1, val2);
    total_sum += test_multi_dim(dst_multi, src1_multi, src2_multi, 
                               val1_multi, val2_multi);
    total_sum += test_complex_pattern(dst1, src1, src2, src3, val1, val2);
    
    /* Use result to prevent optimization */
    sink = total_sum;
    printf("Total checksum: %d\n", total_sum % 1000);
    
    return 0;
}
