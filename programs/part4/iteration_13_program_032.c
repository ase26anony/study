#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
void init_arrays(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        a[i] = (i % 13) * 3;
        b[i] = (i % 7) * 5;
        c[i] = (i % 11) * 7;
        d[i] = (i % 17) * 11;
    }
}

/* Test GT_EXPR (>) pattern - should map to BIT_NOT_EXPR, BIT_AND_EXPR */
__attribute__((noinline))
void test_gt_expr(int *restrict dst, const int *restrict src1, 
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) pattern - should map to BIT_NOT_EXPR, BIT_IOR_EXPR */
__attribute__((noinline))
void test_ge_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) pattern - should map to BIT_NOT_EXPR, BIT_AND_EXPR with swap */
__attribute__((noinline))
void test_lt_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) pattern - should map to BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
__attribute__((noinline))
void test_le_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst, const int *restrict src1,
                            const int *restrict src2, const int *restrict val1,
                            const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; i++) {
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 2; j++) {
            int idx = i*2 + j;
            dst[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
        }
    }
    
    /* Second part with LT_EXPR */
    for (int i = N/2; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8],
                    const int val1[][8], const int val2[][8]) {
    const int rows = N/8;
    
    /* First dimension uses GT_EXPR */
    for (int i = 0; i < rows/2; i++) {
        for (int j = 0; j < 8; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
        }
    }
    
    /* Second half uses LE_EXPR */
    for (int i = rows/2; i < rows; i++) {
        for (int j = 0; j < 8; j++) {
            dst[i][j] = (src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] ^ i;
    }
    return sum;
}

int main(void) {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N];
    ALIGNED int dst_multi[N/8][8];
    ALIGNED int src1_multi[N/8][8], src2_multi[N/8][8];
    ALIGNED int val1_multi[N/8][8], val2_multi[N/8][8];
    
    int total_checksum = 0;
    
    /* Initialize data */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i*8 + j;
            src1_multi[i][j] = src1[idx];
            src2_multi[i][j] = src2[idx];
            val1_multi[i][j] = val1[idx];
            val2_multi[i][j] = val2[idx];
        }
    }
    
    /* Test each comparison operator */
    test_gt_expr(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge_expr(dst2, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt_expr(dst3, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst3, N);
    
    test_le_expr(dst4, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst5, N);
    
    /* Test multi-dimensional with different comparison types */
    test_multi_dim(dst_multi, src1_multi, src2_multi, val1_multi, val2_multi);
    for (int i = 0; i < N/8; i++) {
        total_checksum += compute_checksum(dst_multi[i], 8);
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
