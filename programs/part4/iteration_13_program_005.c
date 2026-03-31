#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR case */
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR case */
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR case */
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR case */
void test_le(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
void test_mixed_comparisons(int *restrict dst, const int *restrict src1,
                            const int *restrict src2, const int *restrict val1,
                            const int *restrict val2, int n) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        /* Inner conditional with LE_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = (val1[i] <= val2[i]) ? src1[i] : src2[i];
        } else {
            dst[i] = (val1[i] > val2[i]) ? src2[i] : src1[i];
        }
    }
}

/* Multi-dimensional array with different comparison types */
void test_multi_dim(int dst[][N], const int src1[][N], const int src2[][N],
                    const int val1[][N], const int val2[][N], int rows) {
    for (int i = 0; i < rows; i++) {
        /* Row processing with GT_EXPR */
        for (int j = 0; j < N; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
        }
        
        /* Column processing with LE_EXPR */
        for (int j = 0; j < N; j++) {
            dst[i][j] += (src1[i][j] <= src2[i][j]) ? i : j;
        }
    }
}

/* Test with OpenMP SIMD pragma */
void test_omp_simd(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, const int *restrict val1,
                   const int *restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma followed by vectorization */
void test_unroll_vectorize(int *restrict dst, const int *restrict src1,
                           const int *restrict src2, const int *restrict val1,
                           const int *restrict val2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
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
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst[N];
    ALIGNED int dst2[N];
    ALIGNED int matrix1[8][N], matrix2[8][N], matrix_val1[8][N], matrix_val2[8][N], matrix_dst[8][N];
    
    int final_checksum = 0;
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = i % 17;
        src2[i] = i % 13;
        val1[i] = i * 3;
        val2[i] = i * 7;
    }
    
    /* Initialize matrices */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < N; j++) {
            matrix1[i][j] = (i * j) % 19;
            matrix2[i][j] = (i * j) % 11;
            matrix_val1[i][j] = i + j * 2;
            matrix_val2[i][j] = i * 3 + j;
        }
    }
    
    /* Test each comparison operator */
    test_gt(dst, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst, N);
    
    test_ge(dst, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst, N);
    
    test_lt(dst, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst, N);
    
    test_le(dst, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst, N);
    
    /* Test multi-dimensional */
    test_multi_dim(matrix_dst, matrix1, matrix2, matrix_val1, matrix_val2, 8);
    for (int i = 0; i < 8; i++) {
        final_checksum += compute_checksum(matrix_dst[i], N);
    }
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst, N);
    
    /* Test with unroll pragma */
    test_unroll_vectorize(dst, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst, N);
    
    /* Additional test with unsigned types to ensure pattern matching */
    {
        ALIGNED unsigned int usrc1[N], usrc2[N], uval1[N], uval2[N], udst[N];
        for (int i = 0; i < N; i++) {
            usrc1[i] = i % 23;
            usrc2[i] = i % 29;
            uval1[i] = i * 5;
            uval2[i] = i * 11;
        }
        
        /* Test unsigned comparisons */
        for (int i = 0; i < N; i++) {
            udst[i] = (usrc1[i] > usrc2[i]) ? uval1[i] : uval2[i];
        }
        for (int i = 0; i < N; i++) {
            final_checksum += udst[i];
        }
    }
    
    /* Complex nested conditional with multiple comparison types */
    for (int i = 0; i < N; i++) {
        int temp;
        if (src1[i] > src2[i]) {
            temp = (val1[i] < val2[i]) ? src1[i] : src2[i];
        } else {
            temp = (val1[i] >= val2[i]) ? src2[i] : src1[i];
        }
        dst2[i] = (temp <= src1[i]) ? temp : src1[i];
    }
    final_checksum += compute_checksum(dst2, N);
    
    printf("Final checksum: %d\n", final_checksum);
    sink = final_checksum; /* Prevent optimization */
    
    return 0;
}
