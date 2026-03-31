#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper functions to prevent dead code elimination */
static int checksum = 0;

/* Initialize arrays with varying patterns to create mix of true/false conditions */
static void init_arrays(int *restrict src1, int *restrict src2, 
                       int *restrict val1, int *restrict val2) {
    for (int i = 0; i < N; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) operator */
__attribute__((noinline))
void test_gt_expr(int *restrict dst, const int *restrict src1, 
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) operator */
__attribute__((noinline))
void test_ge_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) operator */
__attribute__((noinline))
void test_lt_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) operator */
__attribute__((noinline))
void test_le_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context - stresses pattern matching */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst, const int *restrict src1,
                           const int *restrict src2, const int *restrict val1,
                           const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        /* Inner conditional with LE_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        dst[i] = (temp <= src2[i]) ? temp : src2[i];
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8]) {
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            /* GT_EXPR for first dimension, LE_EXPR for second */
            dst[i][j] = (i > j) ? src1[i][j] : 
                        ((src1[i][j] <= src2[i][j]) ? src1[i][j] : src2[i][j]);
        }
    }
}

/* Test with OpenMP SIMD pragma - different vectorization path */
__attribute__((noinline))
void test_omp_simd(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, const int *restrict val1,
                   const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int *restrict dst, const int *restrict src1,
                               const int *restrict src2, const int *restrict val1,
                               const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Compute checksum to prevent dead code elimination */
static void compute_checksum(const int *arr, int size) {
    for (int i = 0; i < size; i++) {
        checksum += arr[i] * (i % 3);
    }
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N];
    ALIGNED int dst_mixed[N];
    ALIGNED int dst_omp[N];
    ALIGNED int dst_unroll[N];
    ALIGNED int dst_multi[N/8][8];
    ALIGNED int src1_multi[N/8][8], src2_multi[N/8][8];
    
    /* Initialize data */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            src1_multi[i][j] = (i * 8 + j) % 17;
            src2_multi[i][j] = (i * 8 + j) % 11;
        }
    }
    
    /* Test each comparison operator */
    test_gt_expr(dst1, src1, src2, val1, val2);
    compute_checksum(dst1, N);
    
    test_ge_expr(dst2, src1, src2, val1, val2);
    compute_checksum(dst2, N);
    
    test_lt_expr(dst3, src1, src2, val1, val2);
    compute_checksum(dst3, N);
    
    test_le_expr(dst4, src1, src2, val1, val2);
    compute_checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed, src1, src2, val1, val2);
    compute_checksum(dst_mixed, N);
    
    /* Test multi-dimensional */
    test_multi_dim(dst_multi, src1_multi, src2_multi);
    for (int i = 0; i < N/8; i++) {
        compute_checksum(dst_multi[i], 8);
    }
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst_omp, src1, src2, val1, val2);
    compute_checksum(dst_omp, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst_unroll, src1, src2, val1, val2);
    compute_checksum(dst_unroll, N);
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
