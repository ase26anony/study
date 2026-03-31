/* test_vectorizable_condition.c
 * Designed to trigger specific uncovered lines in tree-vect-stmts.cc
 * Lines 12216-12233: Mapping comparison operators to bitwise operations
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR (>) -> BIT_NOT_EXPR, BIT_AND_EXPR */
__attribute__((noinline, pure))
int test_gt_expr(int *restrict dst, const int *restrict src1, 
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2) {
    int sum = 0;
    #pragma GCC unroll 4
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* GE_EXPR (>=) -> BIT_NOT_EXPR, BIT_IOR_EXPR */
__attribute__((noinline, pure))
int test_ge_expr(int *restrict dst, const int *restrict src1,
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* LT_EXPR (<) -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
__attribute__((noinline, pure))
int test_lt_expr(int *restrict dst, const int *restrict src1,
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* LE_EXPR (<=) -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
__attribute__((noinline, pure))
int test_le_expr(int *restrict dst, const int *restrict src1,
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2) {
    int sum = 0;
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
int test_mixed_comparisons(int *restrict dst, const int *restrict src1,
                           const int *restrict src2, const int *restrict val1,
                           const int *restrict val2) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; ++i) {
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 2; ++j) {
            int idx = i*2 + j;
            /* First conditional: GT_EXPR */
            int temp = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
            /* Second conditional: LE_EXPR (different type) */
            dst[idx] = (temp <= src1[idx]) ? temp : src2[idx];
            sum += dst[idx];
        }
    }
    return sum;
}

/* Multi-dimensional array with different comparison types per dimension */
__attribute__((noinline))
int test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8]) {
    int sum = 0;
    const int rows = N/8;
    
    /* First dimension uses GT_EXPR */
    for (int i = 0; i < rows; ++i) {
        /* Second dimension uses LE_EXPR */
        #pragma omp simd reduction(+:sum)
        for (int j = 0; j < 8; ++j) {
            dst[i][j] = (i > rows/2) ? 
                       ((src1[i][j] <= src2[i][j]) ? src1[i][j] : src2[i][j]) :
                       ((src1[i][j] > src2[i][j]) ? src2[i][j] : src1[i][j]);
            sum += dst[i][j];
        }
    }
    return sum;
}

/* Complex pattern with multiple conditional assignments */
__attribute__((noinline))
int test_complex_pattern(int *restrict dst, const int *restrict src1,
                         const int *restrict src2, const int *restrict src3) {
    int sum = 0;
    
    /* Mix of GE_EXPR and LT_EXPR in same loop */
    for (int i = 0; i < N; i += 2) {
        /* First: GE_EXPR */
        int cond1 = (src1[i] >= src2[i]) ? src1[i] : src2[i];
        
        /* Second: LT_EXPR (different type, triggers swap) */
        int cond2 = (src3[i] < src1[i]) ? src3[i] : src1[i];
        
        /* Combine with another comparison */
        dst[i] = (cond1 > cond2) ? cond1 : cond2;
        dst[i+1] = (cond1 <= cond2) ? cond1 : cond2;
        
        sum += dst[i] + dst[i+1];
    }
    return sum;
}

int main(void) {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N], src3[N];
    ALIGNED int dst1[N], dst2[N], dst3[N];
    ALIGNED int dst_multi[N/8][8];
    ALIGNED int src_multi1[N/8][8], src_multi2[N/8][8];
    
    int total_sum = 0;
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i * 7) % 31;
        src2[i] = (i * 3) % 31;
        val1[i] = (i * 5) % 127;
        val2[i] = (i * 11) % 127;
        src3[i] = (i * 13) % 31;
    }
    
    for (int i = 0; i < N/8; ++i) {
        for (int j = 0; j < 8; ++j) {
            src_multi1[i][j] = (i * 8 + j) % 31;
            src_multi2[i][j] = (i * 8 + j * 3) % 31;
        }
    }
    
    /* Test each comparison operator individually */
    total_sum += test_gt_expr(dst1, src1, src2, val1, val2);
    total_sum += test_ge_expr(dst2, src1, src2, val1, val2);
    total_sum += test_lt_expr(dst3, src1, src2, val1, val2);
    total_sum += test_le_expr(dst1, src1, src2, val1, val2);
    
    /* Test mixed/nested contexts */
    total_sum += test_mixed_comparisons(dst2, src1, src2, val1, val2);
    total_sum += test_multi_dim(dst_multi, src_multi1, src_multi2);
    total_sum += test_complex_pattern(dst3, src1, src2, src3);
    
    /* Use result to prevent optimization */
    sink = total_sum;
    
    printf("Total checksum: %d\n", total_sum % 1000);
    return 0;
}
