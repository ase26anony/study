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

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_2d(int *restrict dst, const int *restrict src1, const int *restrict src2,
                   const int *restrict val1, const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < M; i++) {
        /* Inner loop with LE_EXPR - stresses pattern matching */
        for (int j = 0; j < M; j++) {
            int idx = i * M + j;
            /* GT_EXPR in outer dimension condition */
            if (src1[idx] > src2[idx]) {
                /* LE_EXPR in inner conditional assignment */
                dst[idx] = (val1[idx] <= val2[idx]) ? val1[idx] : val2[idx];
            } else {
                dst[idx] = 0;
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
        /* Mix of comparison operators in one loop */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        dst[i] = (temp < val1[i]) ? temp : val1[i];
    }
}

/* Test with unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int *restrict dst, const int *restrict src1, 
                                const int *restrict src2, const int *restrict val1,
                                const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        /* GE_EXPR followed by LT_EXPR in same iteration */
        int cond1 = (src1[i] >= src2[i]) ? 1 : 0;
        int cond2 = (val1[i] < val2[i]) ? 1 : 0;
        dst[i] = cond1 + cond2;
    }
}

/* Complex nested conditional with multiple comparison types */
__attribute__((noinline))
void test_complex_nested(int *restrict dst, const int *restrict src1,
                         const int *restrict src2, const int *restrict src3,
                         const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparison operators in ternary chain */
        dst[i] = (src1[i] > src2[i]) ? 
                 ((src3[i] <= val1[i]) ? val1[i] : val2[i]) :
                 ((src3[i] >= val2[i]) ? val2[i] : val1[i]);
    }
}

/* Compute checksum to prevent dead code elimination */
__attribute__((pure))
unsigned long compute_checksum(const int *arr, int size) {
    unsigned long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (unsigned long)arr[i];
    }
    return sum;
}

int main() {
    /* Allocate aligned memory for better vectorization */
    int *src1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *src2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *src3 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *val2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst1 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst2 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst3 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst4 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst5 = (int*)aligned_alloc(64, M * M * sizeof(int));
    int *dst6 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst7 = (int*)aligned_alloc(64, N * sizeof(int));
    int *dst8 = (int*)aligned_alloc(64, N * sizeof(int));
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = i % 7;
        src2[i] = (i % 5) + 1;
        src3[i] = (i % 3) * 2;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
    
    for (int i = 0; i < M * M; i++) {
        int idx = i;
        src1[idx] = idx % 11;
        src2[idx] = (idx % 7) + 2;
        val1[idx] = idx * 2;
        val2[idx] = idx * 3;
    }
    
    unsigned long total_checksum = 0;
    
    /* Test each comparison operator in isolation */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons in 2D array */
    test_mixed_2d(dst5, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst5, M * M);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst6, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst6, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst7, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst7, N);
    
    /* Test complex nested conditionals */
    test_complex_nested(dst8, src1, src2, src3, val1, val2);
    total_checksum += compute_checksum(dst8, N);
    
    printf("Total checksum: %lu\n", total_checksum);
    
    /* Free allocated memory */
    free(src1); free(src2); free(src3);
    free(val1); free(val2);
    free(dst1); free(dst2); free(dst3); free(dst4);
    free(dst5); free(dst6); free(dst7); free(dst8);
    
    return 0;
}
