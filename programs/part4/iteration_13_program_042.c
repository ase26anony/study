#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper functions with no side effects to aid vectorization */
__attribute__((const)) static int get_val1(int i) { return i * 3; }
__attribute__((const)) static int get_val2(int i) { return i * 7; }

/* Test functions for each comparison operator */

/* GT_EXPR case: bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
void test_gt(int* restrict dst, const int* restrict src1, 
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR case: bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
void test_ge(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR case: bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR with swap */
void test_lt(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR case: bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR with swap */
void test_le(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
void test_mixed_comparisons(int* restrict dst, const int* restrict src1,
                            const int* restrict src2, int n) {
    for (int i = 0; i < n; i++) {
        /* Outer condition uses GT_EXPR */
        int temp = (src1[i] > src2[i]) ? src1[i] : src2[i];
        
        /* Inner condition uses LE_EXPR (should trigger swap) */
        dst[i] = (temp <= (src1[i] + src2[i])) ? temp : (src1[i] + src2[i]);
    }
}

/* Multi-dimensional array processing with different comparisons */
void test_multi_dim(int dst[][N], const int src1[][N], 
                    const int src2[][N], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        /* First dimension uses GE_EXPR */
        int row_cond = (i >= rows/2) ? 1 : 0;
        
        for (int j = 0; j < cols; j++) {
            /* Second dimension uses LT_EXPR (should trigger swap) */
            dst[i][j] = (src1[i][j] < src2[i][j]) ? 
                       (src1[i][j] * row_cond) : (src2[i][j] * row_cond);
        }
    }
}

/* Test with OpenMP SIMD pragma - may take different vectorization path */
void test_omp_simd(int* restrict dst, const int* restrict src1,
                   const int* restrict src2, const int* restrict val1,
                   const int* restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with GCC unroll pragma before vectorization */
void test_unroll_then_vectorize(int* restrict dst, const int* restrict src1,
                                const int* restrict src2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ (i & 0xFF);
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N];
    ALIGNED int dst_mixed[N];
    ALIGNED int dst_omp[N];
    ALIGNED int dst_unroll[N];
    
    /* 2D arrays for multi-dimensional test */
    ALIGNED int src1_2d[8][N], src2_2d[8][N], dst_2d[8][N];
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = i % 13;
        src2[i] = i % 7;
        val1[i] = get_val1(i);
        val2[i] = get_val2(i);
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < N; j++) {
            src1_2d[i][j] = (i * j) % 17;
            src2_2d[i][j] = (i + j) % 11;
        }
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed, src1, src2, N);
    total_checksum += compute_checksum(dst_mixed, N);
    
    /* Test multi-dimensional with different comparisons */
    test_multi_dim(dst_2d, src1_2d, src2_2d, 8, N);
    for (int i = 0; i < 8; i++) {
        total_checksum += compute_checksum(dst_2d[i], N);
    }
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst_omp, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst_omp, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst_unroll, src1, src2, N);
    total_checksum += compute_checksum(dst_unroll, N);
    
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
