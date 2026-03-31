/* test_vectorizable_condition.c
 * Designed to trigger specific uncovered lines in tree-vect-stmts.cc
 * Lines 12216-12233: GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR cases
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR case: > operator */
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

/* GE_EXPR case: >= operator */
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

/* LT_EXPR case: < operator */
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

/* LE_EXPR case: <= operator */
__attribute__((noinline, pure))
int test_le(int* restrict dst, const int* restrict src1,
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2) {
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
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
            sum += dst[i][j];
        }
    }
    
    /* Another pass with different comparison */
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < 8; ++j) {
            if (src1[i][j] <= src2[i][j]) {  /* LE_EXPR in condition */
                dst[i][j] = val1[i][j];
            } else {
                dst[i][j] = val2[i][j];
            }
            sum += dst[i][j];
        }
    }
    return sum;
}

/* Test with unsigned types to ensure integer comparisons */
__attribute__((noinline, pure))
unsigned test_unsigned_gt(unsigned* restrict dst, const unsigned* restrict src1,
                         const unsigned* restrict src2, 
                         const unsigned* restrict val1,
                         const unsigned* restrict val2) {
    unsigned sum = 0;
    for (unsigned i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

int main(void) {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N];
    ALIGNED int dst_multi[N/8][8];
    ALIGNED int src1_multi[N/8][8], src2_multi[N/8][8];
    ALIGNED int val1_multi[N/8][8], val2_multi[N/8][8];
    
    ALIGNED unsigned usrc1[N], usrc2[N], uval1[N], uval2[N];
    ALIGNED unsigned udst[N];
    
    long long total_sum = 0;
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i % 7) * 3;
        src2[i] = (i % 5) * 4;
        val1[i] = i * 2;
        val2[i] = i * 3;
        
        usrc1[i] = (i % 11) * 5;
        usrc2[i] = (i % 13) * 3;
        uval1[i] = i * 7;
        uval2[i] = i * 11;
    }
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = i * 8 + j;
            src1_multi[i][j] = (idx % 17) * 2;
            src2_multi[i][j] = (idx % 19) * 3;
            val1_multi[i][j] = idx * 5;
            val2_multi[i][j] = idx * 7;
        }
    }
    
    /* Test each comparison operator in isolation */
    total_sum += test_gt(dst1, src1, src2, val1, val2);
    total_sum += test_ge(dst2, src1, src2, val1, val2);
    total_sum += test_lt(dst1, src1, src2, val1, val2);
    total_sum += test_le(dst2, src1, src2, val1, val2);
    
    /* Test mixed comparisons */
    total_sum += test_mixed_comparisons(dst1, dst2, src1, src2, val1, val2);
    
    /* Test multi-dimensional with different comparisons */
    total_sum += test_multi_dim(dst_multi, src1_multi, src2_multi, 
                               val1_multi, val2_multi);
    
    /* Test unsigned comparison */
    total_sum += test_unsigned_gt(udst, usrc1, usrc2, uval1, uval2);
    
    /* Additional test with runtime size (multiple of vector width) */
    int dynamic_size = 1024;  /* Should be multiple of typical vector width */
    ALIGNED int dyn_src1[dynamic_size], dyn_src2[dynamic_size];
    ALIGNED int dyn_val1[dynamic_size], dyn_val2[dynamic_size];
    ALIGNED int dyn_dst[dynamic_size];
    
    for (int i = 0; i < dynamic_size; ++i) {
        dyn_src1[i] = (i % 23) * 2;
        dyn_src2[i] = (i % 29) * 3;
        dyn_val1[i] = i * 13;
        dyn_val2[i] = i * 17;
    }
    
    /* Test with LT_EXPR on dynamic array */
    int dyn_sum = 0;
    for (int i = 0; i < dynamic_size; ++i) {
        dyn_dst[i] = (dyn_src1[i] < dyn_src2[i]) ? dyn_val1[i] : dyn_val2[i];
        dyn_sum += dyn_dst[i];
    }
    total_sum += dyn_sum;
    
    /* Prevent compiler from optimizing everything away */
    sink = total_sum % 1000;
    
    printf("Total checksum: %lld\n", total_sum);
    printf("Sink value: %d\n", sink);
    
    return 0;
}
