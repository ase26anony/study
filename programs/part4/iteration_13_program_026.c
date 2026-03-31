/* test_vectorizable_condition.c
 * 
 * This program is designed to trigger specific uncovered lines in GCC's
 * tree-vect-stmts.cc file, specifically in the vectorizable_condition
 * function for GT_EXPR, GE_EXPR, LT_EXPR, and LE_EXPR comparisons.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR (>) test */
__attribute__((noinline))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR (>=) test */
__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR (<) test */
__attribute__((noinline))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR (<=) test */
__attribute__((noinline))
void test_le(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_nested(int *restrict dst, const int *restrict src1,
                       const int *restrict src2, const int *restrict val1,
                       const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        /* Outer condition uses GT_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner condition uses LE_EXPR */
        dst[i] = (temp <= src1[i]) ? temp : src2[i];
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[M][M], const int src1[M][M], 
                    const int src2[M][M]) {
    /* Row processing with GT_EXPR */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? src1[i][j] : src2[i][j];
        }
    }
    
    /* Column processing with LE_EXPR */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < M; i++) {
            dst[i][j] = (dst[i][j] <= src1[i][j]) ? dst[i][j] : src1[i][j];
        }
    }
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, const int *restrict val1,
                   const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with GCC unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_vectorize(int *restrict dst, const int *restrict src1,
                           const int *restrict src2, const int *restrict val1,
                           const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unsigned integers */
__attribute__((noinline))
void test_unsigned(unsigned int *restrict dst, 
                   const unsigned int *restrict src1,
                   const unsigned int *restrict src2,
                   const unsigned int *restrict val1,
                   const unsigned int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Allocate and initialize arrays */
    int *src1 = malloc(N * sizeof(int));
    int *src2 = malloc(N * sizeof(int));
    int *val1 = malloc(N * sizeof(int));
    int *val2 = malloc(N * sizeof(int));
    int *dst = malloc(N * sizeof(int));
    
    unsigned int *usrc1 = malloc(N * sizeof(unsigned int));
    unsigned int *usrc2 = malloc(N * sizeof(unsigned int));
    unsigned int *uval1 = malloc(N * sizeof(unsigned int));
    unsigned int *uval2 = malloc(N * sizeof(unsigned int));
    unsigned int *udst = malloc(N * sizeof(unsigned int));
    
    int dst2d[M][M];
    int src1_2d[M][M];
    int src2_2d[M][M];
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = i % 7;
        src2[i] = (i * 3) % 11;
        val1[i] = i * 2;
        val2[i] = i * 3;
        
        usrc1[i] = (unsigned int)(i % 13);
        usrc2[i] = (unsigned int)((i * 5) % 17);
        uval1[i] = (unsigned int)(i * 4);
        uval2[i] = (unsigned int)(i * 5);
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            src1_2d[i][j] = (i * M + j) % 19;
            src2_2d[i][j] = ((i * M + j) * 7) % 23;
        }
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    test_ge(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    test_lt(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    test_le(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test mixed nested comparisons */
    test_mixed_nested(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test multi-dimensional arrays */
    test_multi_dim(dst2d, src1_2d, src2_2d);
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            total_checksum += dst2d[i][j];
        }
    }
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test with unroll pragma */
    test_unroll_vectorize(dst, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst, N);
    
    /* Test with unsigned integers */
    test_unsigned(udst, usrc1, usrc2, uval1, uval2);
    for (int i = 0; i < N; i++) {
        total_checksum += (int)udst[i];
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(val1);
    free(val2);
    free(dst);
    free(usrc1);
    free(usrc2);
    free(uval1);
    free(uval2);
    free(udst);
    
    return 0;
}
