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
        #pragma GCC unroll 4
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* Outer dimension uses GT, inner dimension uses LE */
            dst[idx] = (i > rows/2) ? 
                       ((src1[idx] <= src2[idx]) ? src1[idx] : src2[idx]) :
                       ((src1[idx] > src2[idx]) ? src2[idx] : src1[idx]);
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
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unsigned types to ensure different type handling */
__attribute__((noinline))
void test_unsigned(unsigned int *restrict dst, 
                   const unsigned int *restrict src1,
                   const unsigned int *restrict src2,
                   const unsigned int *restrict val1,
                   const unsigned int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Complex pattern with multiple conditions */
__attribute__((noinline))
void test_complex_pattern(int *restrict dst, const int *restrict a,
                          const int *restrict b, const int *restrict c,
                          int n) {
    for (int i = 0; i < n; i++) {
        /* Mix of GT and LE in same expression */
        dst[i] = (a[i] > b[i]) ? 
                 ((b[i] <= c[i]) ? a[i] + b[i] : a[i] - c[i]) :
                 ((b[i] > c[i]) ? b[i] + c[i] : a[i] + c[i]);
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ (i & 0xFF);
    }
    return sum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int *src1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *src2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst3 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst4 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst_mixed = (int*)aligned_alloc(64, M * M * sizeof(int));
    int *src1_2d = (int*)aligned_alloc(64, M * M * sizeof(int));
    int *src2_2d = (int*)aligned_alloc(64, M * M * sizeof(int));
    
    unsigned int *usrc1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *usrc2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *uval1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *uval2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int *udst = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = (i * 7) % 19;
        src2[i] = (i * 3) % 17;
        val1[i] = i * 2;
        val2[i] = i * 3;
        
        usrc1[i] = (i * 11) % 23;
        usrc2[i] = (i * 5) % 29;
        uval1[i] = i * 4;
        uval2[i] = i * 5;
    }
    
    for (int i = 0; i < M * M; i++) {
        src1_2d[i] = (i * 13) % 31;
        src2_2d[i] = (i * 17) % 37;
    }
    
    int checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparison types in 2D */
    test_mixed_2d(dst_mixed, src1_2d, src2_2d, M, M);
    checksum += compute_checksum(dst_mixed, M * M);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst1, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst1, N);
    
    /* Test unsigned comparisons */
    test_unsigned(udst, usrc1, usrc2, uval1, uval2, N);
    for (int i = 0; i < N; i++) {
        checksum += (int)udst[i] ^ (i & 0xFF);
    }
    
    /* Test complex pattern */
    test_complex_pattern(dst1, src1, src2, val1, N);
    checksum += compute_checksum(dst1, N);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Free allocated memory */
    free(src1); free(src2); free(val1); free(val2);
    free(dst1); free(dst2); free(dst3); free(dst4);
    free(dst_mixed); free(src1_2d); free(src2_2d);
    free(usrc1); free(usrc2); free(uval1); free(uval2); free(udst);
    
    return 0;
}
