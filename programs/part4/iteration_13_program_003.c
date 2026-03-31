#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper functions with attributes to aid vectorization analysis */
__attribute__((const)) static int pure_rand(int seed) {
    return (seed * 1103515245 + 12345) & 0x7fffffff;
}

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
__attribute__((noinline)) 
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR (>=) */
__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR (<) */
__attribute__((noinline))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR (<=) */
__attribute__((noinline))
void test_le(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst, 
                           const int *restrict src1,
                           const int *restrict src2,
                           const int *restrict val1,
                           const int *restrict val2,
                           int n, int m) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; ++i) {
        int base = i * m;
        /* Inner loop with LE_EXPR - creates different pattern */
        for (int j = 0; j < m; ++j) {
            int idx = base + j;
            /* First conditional with GT_EXPR */
            int temp = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
            /* Second conditional with LE_EXPR in same expression */
            dst[idx] = (src1[idx] <= src2[idx]) ? temp : (val1[idx] + val2[idx]);
        }
    }
}

/* Multi-dimensional array processing with different comparisons */
__attribute__((noinline))
void test_multi_dim(int dst[M][M], const int src1[M][M],
                   const int src2[M][M], const int val1[M][M],
                   const int val2[M][M]) {
    /* Row processing with GT_EXPR */
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
        }
    }
    
    /* Column processing with LE_EXPR */
    for (int j = 0; j < M; ++j) {
        for (int i = 0; i < M; ++i) {
            dst[i][j] = (src1[i][j] <= src2[i][j]) ? dst[i][j] : (val1[i][j] - val2[i][j]);
        }
    }
}

/* Complex pattern with multiple comparisons in same loop */
__attribute__((noinline))
void test_complex_pattern(int *restrict dst,
                         const int *restrict src1,
                         const int *restrict src2,
                         const int *restrict src3,
                         const int *restrict val1,
                         const int *restrict val2,
                         int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        /* Chain of comparisons: GT, GE, LT, LE */
        int cond1 = (src1[i] > src2[i]) ? val1[i] : val2[i];
        int cond2 = (src1[i] >= src3[i]) ? cond1 : (val1[i] * 2);
        int cond3 = (src2[i] < src3[i]) ? cond2 : (val2[i] * 2);
        dst[i] = (src3[i] <= src1[i]) ? cond3 : (cond1 + cond2);
    }
}

/* Compute checksum to prevent dead code elimination */
__attribute__((noinline))
unsigned long long compute_checksum(const int *data, int n) {
    unsigned long long sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += (unsigned long long)data[i];
    }
    return sum;
}

int main() {
    /* Allocate and initialize arrays */
    int *src1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *src2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *src3 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Multi-dimensional arrays */
    int (*md_src1)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    int (*md_src2)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    int (*md_val1)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    int (*md_val2)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    int (*md_dst)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = pure_rand(i) % 100;
        src2[i] = pure_rand(i + N) % 100;
        src3[i] = pure_rand(i + 2*N) % 100;
        val1[i] = pure_rand(i + 3*N) % 1000;
        val2[i] = pure_rand(i + 4*N) % 1000;
    }
    
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < M; ++j) {
            md_src1[i][j] = pure_rand(i * M + j) % 200;
            md_src2[i][j] = pure_rand(i * M + j + M*M) % 200;
            md_val1[i][j] = pure_rand(i * M + j + 2*M*M) % 500;
            md_val2[i][j] = pure_rand(i * M + j + 3*M*M) % 500;
        }
    }
    
    unsigned long long total_checksum = 0;
    
    /* Test each comparison operator */
    printf("Testing GT_EXPR (>)\n");
    test_gt(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    printf("Testing GE_EXPR (>=)\n");
    test_ge(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    printf("Testing LT_EXPR (<)\n");
    test_lt(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    printf("Testing LE_EXPR (<=)\n");
    test_le(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    printf("Testing mixed comparisons\n");
    test_mixed_comparisons(dst, src1, src2, val1, val2, 8, 128);
    total_checksum += compute_checksum(dst, 1024);
    
    printf("Testing multi-dimensional\n");
    test_multi_dim(md_dst, md_src1, md_src2, md_val1, md_val2);
    total_checksum += compute_checksum(&md_dst[0][0], M * M);
    
    printf("Testing complex pattern\n");
    test_complex_pattern(dst, src1, src2, src3, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    printf("Final checksum: %llu\n", total_checksum);
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(src3);
    free(val1);
    free(val2);
    free(dst);
    free(md_src1);
    free(md_src2);
    free(md_val1);
    free(md_val2);
    free(md_dst);
    
    return 0;
}
