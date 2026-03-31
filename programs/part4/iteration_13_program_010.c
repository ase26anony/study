#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR case */
__attribute__((noinline, pure))
void test_gt(int* restrict dst, const int* restrict src1, 
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR case */
__attribute__((noinline, pure))
void test_ge(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR case */
__attribute__((noinline, pure))
void test_lt(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR case */
__attribute__((noinline, pure))
void test_le(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed(int* restrict dst, const int* restrict src1,
                const int* restrict src2, const int* restrict val1,
                const int* restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; ++i) {
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 2; ++j) {
            int idx = i*2 + j;
            dst[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
        }
    }
    
    /* Second part with LE_EXPR */
    for (int i = N/2; i < N; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int* restrict dst, const int* restrict src1,
                   const int* restrict src2, const int* restrict val1,
                   const int* restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma followed by vectorization */
__attribute__((noinline))
void test_unroll_vectorize(int* restrict dst, const int* restrict src1,
                           const int* restrict src2, const int* restrict val1,
                           const int* restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Multi-dimensional array test with different comparisons per dimension */
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], 
                    const int src2[][8], const int val1[][8],
                    const int val2[][8]) {
    for (int i = 0; i < N/8; ++i) {
        for (int j = 0; j < 8; ++j) {
            /* GT_EXPR for rows, LE_EXPR for columns */
            if (i % 2 == 0) {
                dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
            } else {
                dst[i][j] = (src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j];
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(const int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N];
    ALIGNED int src2[N];
    ALIGNED int val1[N];
    ALIGNED int val2[N];
    ALIGNED int dst[N];
    
    /* Multi-dimensional arrays */
    ALIGNED int src1_md[N/8][8];
    ALIGNED int src2_md[N/8][8];
    ALIGNED int val1_md[N/8][8];
    ALIGNED int val2_md[N/8][8];
    ALIGNED int dst_md[N/8][8];
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i * 7) % 31;
        src2[i] = (i * 3) % 31;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
    
    for (int i = 0; i < N/8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = i*8 + j;
            src1_md[i][j] = (idx * 11) % 31;
            src2_md[i][j] = (idx * 5) % 31;
            val1_md[i][j] = idx * 2;
            val2_md[i][j] = idx * 3;
        }
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    test_ge(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    test_lt(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    test_le(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    /* Test mixed comparisons */
    test_mixed(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    /* Test with unroll pragma */
    test_unroll_vectorize(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    /* Test multi-dimensional */
    test_multi_dim(dst_md, src1_md, src2_md, val1_md, val2_md);
    for (int i = 0; i < N/8; ++i) {
        total_checksum += checksum(dst_md[i], 8);
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
