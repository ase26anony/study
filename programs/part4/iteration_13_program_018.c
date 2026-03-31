#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR case */
void test_gt(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR case */
void test_ge(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR case */
void test_lt(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR case */
void test_le(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
void test_mixed(int *restrict dst, const int *restrict src1, const int *restrict src2,
                const int *restrict val1, const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        /* Inner conditional with LE_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = (src1[i] <= val1[i]) ? val1[i] : val2[i];
        } else {
            dst[i] = (src2[i] <= val2[i]) ? val2[i] : val1[i];
        }
    }
}

/* Test with OpenMP SIMD pragma */
void test_omp_simd(int *restrict dst, const int *restrict src1, const int *restrict src2,
                   const int *restrict val1, const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma followed by vectorization */
void test_unroll_then_vectorize(int *restrict dst, const int *restrict src1, 
                                const int *restrict src2, const int *restrict val1,
                                const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Multi-dimensional array test with different comparisons per dimension */
void test_multi_dim(int dst[][N], const int src1[][N], const int src2[][N],
                    const int val1[][N], const int val2[][N]) {
    for (int i = 0; i < N/16; i++) {
        /* First dimension uses GT_EXPR */
        for (int j = 0; j < N/16; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
        }
        /* Second dimension uses LE_EXPR */
        for (int j = N/16; j < N/8; j++) {
            dst[i][j] = (src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] ^ (i & 0xFF);
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst[N];
    ALIGNED int dst2[N][N];
    ALIGNED int src1_2d[N][N], src2_2d[N][N], val1_2d[N][N], val2_2d[N][N];
    
    int total_checksum = 0;
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = (i * 7) % 19;
        src2[i] = (i * 3) % 17;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            src1_2d[i][j] = (i * 13 + j * 7) % 23;
            src2_2d[i][j] = (i * 11 + j * 5) % 19;
            val1_2d[i][j] = i + j * 2;
            val2_2d[i][j] = i * 3 + j;
        }
    }
    
    /* Test each comparison operator */
    test_gt(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    test_ge(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    test_lt(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    test_le(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test mixed comparisons */
    test_mixed(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test multi-dimensional */
    test_multi_dim(dst2, src1_2d, src2_2d, val1_2d, val2_2d);
    for (int i = 0; i < N/16; i++) {
        total_checksum += compute_checksum(dst2[i], N/8);
    }
    
    /* Use sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
