#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper functions with restrict to avoid aliasing issues */
__attribute__((noinline))
void test_gt(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        /* GT_EXPR: > operator */
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        /* GE_EXPR: >= operator */
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline))
void test_lt(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        /* LT_EXPR: < operator */
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline))
void test_le(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        /* LE_EXPR: <= operator */
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested contexts */
__attribute__((noinline))
void test_mixed_2d(int *restrict dst, const int *restrict src1, const int *restrict src2,
                   const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            /* Outer dimension uses GT_EXPR, inner uses LE_EXPR */
            if (src1[idx] > src2[idx]) {
                dst[idx] = (val1[idx] <= val2[idx]) ? src1[idx] : src2[idx];
            } else {
                dst[idx] = (val1[idx] <= val2[idx]) ? val1[idx] : val2[idx];
            }
        }
    }
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int *restrict dst, const int *restrict src1, const int *restrict src2,
                   const int *restrict val1, const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        /* Mix of comparison operators */
        dst[i] = (src1[i] > src2[i]) ? 
                 ((val1[i] < val2[i]) ? src1[i] : val1[i]) :
                 ((val1[i] >= val2[i]) ? src2[i] : val2[i]);
    }
}

/* Test with GCC unroll pragma */
__attribute__((noinline))
void test_unroll_vectorize(int *restrict dst, const int *restrict src1, const int *restrict src2,
                           const int *restrict val1, const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        /* LE_EXPR followed by GT_EXPR in same iteration */
        int temp = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        dst[i] = (temp > src1[i]) ? temp : src1[i];
    }
}

/* Compute checksum to prevent dead code elimination */
__attribute__((const))
int compute_checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
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
    int *dst5 = (int*)aligned_alloc(64, M * M * sizeof(int));
    int *dst6 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst7 = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = (i * 7) % 19;
        src2[i] = (i * 3) % 17;
        val1[i] = (i * 5) % 23;
        val2[i] = (i * 11) % 29;
    }
    
    for (int i = 0; i < M * M; i++) {
        int idx = i;
        src1[idx] = (idx * 13) % 31;
        src2[idx] = (idx * 17) % 37;
        val1[idx] = (idx * 19) % 41;
        val2[idx] = (idx * 23) % 43;
    }
    
    int checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2);
    checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons in 2D */
    test_mixed_2d(dst5, src1, src2, val1, val2);
    checksum += compute_checksum(dst5, M * M);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst6, src1, src2, val1, val2);
    checksum += compute_checksum(dst6, N);
    
    /* Test with unroll pragma */
    test_unroll_vectorize(dst7, src1, src2, val1, val2);
    checksum += compute_checksum(dst7, N);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src1); free(src2); free(val1); free(val2);
    free(dst1); free(dst2); free(dst3); free(dst4);
    free(dst5); free(dst6); free(dst7);
    
    return 0;
}
