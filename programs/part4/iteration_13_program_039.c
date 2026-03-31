#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper functions with restrict to avoid aliasing issues */
__attribute__((noinline))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline))
void test_le(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_2d(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Outer dimension uses GT_EXPR, inner uses LE_EXPR */
            dst[idx] = (i > rows/2) ? 
                       ((src1[idx] <= src2[idx]) ? src1[idx] : src2[idx]) :
                       ((src1[idx] > src2[idx]) ? src2[idx] : src1[idx]);
        }
    }
}

/* Test with unsigned types to ensure different type conversions */
__attribute__((noinline))
void test_unsigned_gt(unsigned int *restrict dst,
                      const unsigned int *restrict src1,
                      const unsigned int *restrict src2,
                      const unsigned int *restrict val1,
                      const unsigned int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with explicit SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, const int *restrict val1,
                   const int *restrict val2, int n) {
#pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int *restrict dst, const int *restrict src1,
                                const int *restrict src2, int n) {
#pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? src1[i] : src2[i];
    }
}

/* Complex nested condition with multiple comparison types */
__attribute__((noinline))
void test_complex_nested(int *restrict dst, const int *restrict a,
                         const int *restrict b, const int *restrict c,
                         const int *restrict d, int n) {
    for (int i = 0; i < n; i++) {
        /* Mix GT_EXPR and LE_EXPR in same expression */
        dst[i] = (a[i] > b[i]) ? 
                 ((c[i] <= d[i]) ? a[i] + c[i] : b[i] + d[i]) :
                 ((c[i] > d[i]) ? a[i] - c[i] : b[i] - d[i]);
    }
}

/* Compute checksum to prevent dead code elimination */
__attribute__((const))
int compute_checksum(const int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ (i & 0xFF);
    }
    return sum;
}

int main(void) {
    /* Allocate aligned memory for better vectorization */
    int *src1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *src2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst3 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst4 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst5 = (int*)aligned_alloc(64, N * M * sizeof(int));
    
    unsigned int *usrc1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *usrc2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *uval1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *uval2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *udst = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = i % 7;
        src2[i] = (i % 5) + 1;
        val1[i] = i * 3;
        val2[i] = i * 5;
        
        usrc1[i] = (i % 11) * 2;
        usrc2[i] = (i % 13) + 1;
        uval1[i] = i * 7;
        uval2[i] = i * 11;
    }
    
    for (int i = 0; i < N * M; i++) {
        dst5[i] = 0;
    }
    
    int checksum = 0;
    
    /* Test each comparison operator in vectorizable loops */
    test_gt(dst1, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparison types in 2D context */
    test_mixed_2d(dst5, src1, src2, N, M);
    checksum += compute_checksum(dst5, N * M);
    
    /* Test unsigned comparison */
    test_unsigned_gt(udst, usrc1, usrc2, uval1, uval2, N);
    for (int i = 0; i < N; i++) {
        checksum += (int)udst[i];
    }
    
    /* Test with OpenMP SIMD pragma */
    test_omp_simd(dst1, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst1, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst2, src1, src2, N);
    checksum += compute_checksum(dst2, N);
    
    /* Test complex nested conditions */
    test_complex_nested(dst3, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst3, N);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src1); free(src2); free(val1); free(val2);
    free(dst1); free(dst2); free(dst3); free(dst4); free(dst5);
    free(usrc1); free(usrc2); free(uval1); free(uval2); free(udst);
    
    return 0;
}
