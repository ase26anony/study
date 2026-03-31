#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
static void init_arrays(int *a, int *b, int *c, int *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (i % 13) * 3;
        b[i] = (i % 7) * 5;
        c[i] = (i % 19) * 2;
        d[i] = (i % 11) * 4;
    }
}

/* Test GT_EXPR (>) pattern */
__attribute__((noinline))
void test_gt_expr(int *restrict dst, const int *restrict src1, 
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) pattern */
__attribute__((noinline))
void test_ge_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) pattern */
__attribute__((noinline))
void test_lt_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) pattern */
__attribute__((noinline))
void test_le_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst, const int *restrict src1,
                           const int *restrict src2, const int *restrict val1,
                           const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        /* Outer condition uses GT_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner condition uses LE_EXPR */
        dst[i] = (temp <= src2[i]) ? src1[i] : temp;
    }
}

/* 2D array processing with different comparison types */
__attribute__((noinline))
void test_2d_mixed(int dst[M][M], const int src1[M][M], const int src2[M][M]) {
    /* Row-wise: GT_EXPR */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? src1[i][j] : src2[i][j];
        }
    }
    
    /* Column-wise: LE_EXPR (different pattern) */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < M; i++) {
            dst[i][j] = (dst[i][j] <= src1[i][j]) ? dst[i][j] + 1 : dst[i][j];
        }
    }
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, const int *restrict val1,
                   const int *restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];  /* LT_EXPR */
    }
}

/* Test with unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int *restrict dst, const int *restrict src1,
                               const int *restrict src2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? src1[i] : src2[i];  /* GE_EXPR */
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ i;
    }
    return sum;
}

int main(void) {
    /* Allocate aligned arrays for better vectorization */
    int *src1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *src2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst3 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst4 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst5 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst6 = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* 2D arrays */
    int (*arr2d_dst)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    int (*arr2d_src1)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    int (*arr2d_src2)[M] = (int(*)[M])aligned_alloc(64, M * M * sizeof(int));
    
    if (!src1 || !src2 || !val1 || !val2 || !dst1 || !dst2 || !dst3 || 
        !dst4 || !dst5 || !dst6 || !arr2d_dst || !arr2d_src1 || !arr2d_src2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(src1, src2, val1, val2, N);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            arr2d_src1[i][j] = (i * M + j) % 17;
            arr2d_src2[i][j] = (i * M + j) % 13;
        }
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator in vectorizable loops */
    test_gt_expr(dst1, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge_expr(dst2, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt_expr(dst3, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst3, N);
    
    test_le_expr(dst4, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst5, N);
    
    /* Test 2D mixed comparisons */
    test_2d_mixed(arr2d_dst, arr2d_src1, arr2d_src2);
    total_checksum += compute_checksum((int*)arr2d_dst, M * M);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst6, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst6, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst1, src1, src2, N);
    total_checksum += compute_checksum(dst1, N);
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(src1); free(src2); free(val1); free(val2);
    free(dst1); free(dst2); free(dst3); free(dst4);
    free(dst5); free(dst6);
    free(arr2d_dst); free(arr2d_src1); free(arr2d_src2);
    
    return 0;
}
