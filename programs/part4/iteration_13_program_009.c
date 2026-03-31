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
void test_mixed(int *restrict dst, const int *restrict src1,
                const int *restrict src2, const int *restrict val1,
                const int *restrict val2, int n, int m) {
    for (int i = 0; i < n; i++) {
        /* Outer loop with GT_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < m; j++) {
            dst[i * m + j] = (src1[i] <= src2[j]) ? temp : val2[j];
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
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with GCC unroll pragma followed by vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int *restrict dst, const int *restrict src1,
                                const int *restrict src2, const int *restrict val1,
                                const int *restrict val2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Multi-dimensional array with different comparison types per dimension */
__attribute__((noinline))
void test_multi_dim(int *restrict dst, const int *restrict src1,
                    const int *restrict src2, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            /* GT_EXPR for row dimension, LE_EXPR for column dimension */
            dst[idx] = (src1[i] > src2[i]) ? 
                       ((src1[j] <= src2[j]) ? src1[idx] : src2[idx]) :
                       ((src1[j] > src2[j]) ? src2[idx] : src1[idx]);
        }
    }
}

/* Compute checksum to prevent dead code elimination */
unsigned long long compute_checksum(const int *arr, int n) {
    unsigned long long sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (unsigned long long)arr[i];
    }
    return sum;
}

int main() {
    /* Allocate and initialize arrays with varying patterns */
    int *src1 = malloc(N * sizeof(int));
    int *src2 = malloc(N * sizeof(int));
    int *val1 = malloc(N * sizeof(int));
    int *val2 = malloc(N * sizeof(int));
    int *dst1 = malloc(N * sizeof(int));
    int *dst2 = malloc(N * sizeof(int));
    int *dst3 = malloc(N * sizeof(int));
    int *dst4 = malloc(N * sizeof(int));
    int *dst_mixed = malloc(N * M * sizeof(int));
    int *dst_multi = malloc(N * N * sizeof(int));
    
    /* Initialize with varying data to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = i % 7;          /* Pattern: 0,1,2,3,4,5,6,0,1,... */
        src2[i] = (i * 3) % 11;   /* Pattern: 0,3,6,9,1,4,7,10,2,... */
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
    
    for (int i = 0; i < N * M; i++) {
        dst_mixed[i] = 0;
    }
    
    for (int i = 0; i < N * N; i++) {
        dst_multi[i] = 0;
    }
    
    unsigned long long total_checksum = 0;
    
    /* Test each comparison operator in separate vectorizable loops */
    test_gt(dst1, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparison types */
    test_mixed(dst_mixed, src1, src2, val1, val2, N, M);
    total_checksum += compute_checksum(dst_mixed, N * M);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst1, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst1, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst2, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst2, N);
    
    /* Test multi-dimensional with different comparison types */
    test_multi_dim(dst_multi, src1, src2, N, N);
    total_checksum += compute_checksum(dst_multi, N * N);
    
    printf("Total checksum: %llu\n", total_checksum);
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(val1);
    free(val2);
    free(dst1);
    free(dst2);
    free(dst3);
    free(dst4);
    free(dst_mixed);
    free(dst_multi);
    
    return 0;
}
