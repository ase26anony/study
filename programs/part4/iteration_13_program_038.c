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
                 ((a[i] <= b[i]) ? c[i] - d[i] : d[i] - c[i]);
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(const int *arr, int n) {
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
    int *dst = (int*)aligned_alloc(64, N * sizeof(int));
    
    unsigned int *usrc1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *usrc2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *uval1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *uval2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *udst = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    
    int *src2d = (int*)aligned_alloc(64, M * M * sizeof(int));
    int *dst2d = (int*)aligned_alloc(64, M * M * sizeof(int));
    
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
    
    for (int i = 0; i < M * M; i++) {
        src2d[i] = (i % 17) - 8;
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator in isolation */
    test_gt(dst, src1, src2, val1, val2, N);
    total_checksum += checksum(dst, N);
    
    test_ge(dst, src1, src2, val1, val2, N);
    total_checksum += checksum(dst, N);
    
    test_lt(dst, src1, src2, val1, val2, N);
    total_checksum += checksum(dst, N);
    
    test_le(dst, src1, src2, val1, val2, N);
    total_checksum += checksum(dst, N);
    
    /* Test unsigned comparison */
    test_unsigned_gt(udst, usrc1, usrc2, uval1, uval2, N);
    for (int i = 0; i < N; i++) {
        total_checksum += udst[i] ^ (i & 0xFF);
    }
    
    /* Test mixed comparisons in 2D */
    test_mixed_2d(dst2d, src2d, src2d, M, M);
    total_checksum += checksum(dst2d, M * M);
    
    /* Test with OpenMP SIMD pragma */
    test_omp_simd(dst, src1, src2, val1, val2, N);
    total_checksum += checksum(dst, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst, src1, src2, N);
    total_checksum += checksum(dst, N);
    
    /* Test complex nested conditions */
    test_complex_nested(dst, src1, src2, val1, val2, N);
    total_checksum += checksum(dst, N);
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Free allocated memory */
    free(src1); free(src2); free(val1); free(val2); free(dst);
    free(usrc1); free(usrc2); free(uval1); free(uval2); free(udst);
    free(src2d); free(dst2d);
    
    return 0;
}
