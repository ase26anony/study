#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
__attribute__((noinline))
void init_arrays(int *restrict src1, int *restrict src2, 
                 int *restrict val1, int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) pattern */
__attribute__((noinline, pure))
int test_gt_expr(int *restrict dst, const int *restrict src1, 
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Test GE_EXPR (>=) pattern */
__attribute__((noinline, pure))
int test_ge_expr(int *restrict dst, const int *restrict src1,
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Test LT_EXPR (<) pattern */
__attribute__((noinline, pure))
int test_lt_expr(int *restrict dst, const int *restrict src1,
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Test LE_EXPR (<=) pattern */
__attribute__((noinline, pure))
int test_le_expr(int *restrict dst, const int *restrict src1,
                 const int *restrict src2, const int *restrict val1,
                 const int *restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
int test_mixed_comparisons(int *restrict dst1, int *restrict dst2,
                           const int *restrict src1, const int *restrict src2,
                           const int *restrict val1, const int *restrict val2,
                           int n, int m) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            dst1[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
            sum += dst1[idx];
        }
        
        /* Another conditional with LE_EXPR in same iteration */
        #pragma omp simd
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            dst2[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
            sum += dst2[idx];
        }
    }
    return sum;
}

/* Test with #pragma GCC unroll before vectorization */
__attribute__((noinline))
int test_with_unroll(int *restrict dst, const int *restrict src1,
                     const int *restrict src2, const int *restrict val1,
                     const int *restrict val2, int n) {
    int sum = 0;
    
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    
    /* Follow with another loop using different comparison */
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    
    return sum;
}

/* Multi-dimensional array processing with different comparisons */
__attribute__((noinline))
int test_multi_dim(int dst[M][M], const int src1[M][M], 
                   const int src2[M][M], const int val1[M][M],
                   const int val2[M][M]) {
    int sum = 0;
    
    /* Row-wise with GT_EXPR */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
            sum += dst[i][j];
        }
    }
    
    /* Column-wise with LE_EXPR */
    for (int j = 0; j < M; j++) {
        #pragma omp simd
        for (int i = 0; i < M; i++) {
            dst[i][j] = (src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j];
            sum += dst[i][j];
        }
    }
    
    return sum;
}

int main(void) {
    /* Allocate aligned arrays for better vectorization */
    int *src1 = __builtin_assume_aligned(malloc(N * sizeof(int)), 32);
    int *src2 = __builtin_assume_aligned(malloc(N * sizeof(int)), 32);
    int *val1 = __builtin_assume_aligned(malloc(N * sizeof(int)), 32);
    int *val2 = __builtin_assume_aligned(malloc(N * sizeof(int)), 32);
    int *dst  = __builtin_assume_aligned(malloc(N * sizeof(int)), 32);
    
    int *dst2 = __builtin_assume_aligned(malloc(N * M * sizeof(int)), 32);
    int *src1_2d = __builtin_assume_aligned(malloc(N * M * sizeof(int)), 32);
    int *src2_2d = __builtin_assume_aligned(malloc(N * M * sizeof(int)), 32);
    int *val1_2d = __builtin_assume_aligned(malloc(N * M * sizeof(int)), 32);
    int *val2_2d = __builtin_assume_aligned(malloc(N * M * sizeof(int)), 32);
    
    int dst_md[M][M] __attribute__((aligned(32)));
    int src1_md[M][M] __attribute__((aligned(32)));
    int src2_md[M][M] __attribute__((aligned(32)));
    int val1_md[M][M] __attribute__((aligned(32)));
    int val2_md[M][M] __attribute__((aligned(32)));
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2, N);
    
    for (int i = 0; i < N * M; i++) {
        src1_2d[i] = (i % 17) * 2;
        src2_2d[i] = (i % 11) * 3;
        val1_2d[i] = i;
        val2_2d[i] = i * 2;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            src1_md[i][j] = (i * M + j) % 19;
            src2_md[i][j] = (i * M + j) % 13;
            val1_md[i][j] = i * 10 + j;
            val2_md[i][j] = i * 5 + j * 3;
        }
    }
    
    int total_sum = 0;
    
    /* Test each comparison operator individually */
    total_sum += test_gt_expr(dst, src1, src2, val1, val2, N);
    total_sum += test_ge_expr(dst, src1, src2, val1, val2, N);
    total_sum += test_lt_expr(dst, src1, src2, val1, val2, N);
    total_sum += test_le_expr(dst, src1, src2, val1, val2, N);
    
    /* Test mixed comparisons in nested context */
    total_sum += test_mixed_comparisons(dst2, dst2 + N*M/2, 
                                       src1_2d, src2_2d, 
                                       val1_2d, val2_2d, N/4, M);
    
    /* Test with unroll pragma */
    total_sum += test_with_unroll(dst, src1, src2, val1, val2, N);
    
    /* Test multi-dimensional */
    total_sum += test_multi_dim(dst_md, src1_md, src2_md, val1_md, val2_md);
    
    /* Use volatile sink to prevent optimization */
    sink = total_sum;
    
    printf("Total checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(val1);
    free(val2);
    free(dst);
    free(dst2);
    free(src1_2d);
    free(src2_2d);
    free(val1_2d);
    free(val2_2d);
    
    return 0;
}
