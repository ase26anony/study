#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns to create mixed conditions */
void init_arrays(int* ALIGNED src1, int* ALIGNED src2, 
                 int* ALIGNED val1, int* ALIGNED val2, int n) {
    for (int i = 0; i < n; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) */
__attribute__((noinline, pure))
void test_gt(int* restrict dst, const int* restrict src1, 
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
void test_ge(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) */
__attribute__((noinline, pure))
void test_lt(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) */
__attribute__((noinline, pure))
void test_le(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_nested(int* restrict dst, const int* restrict src1,
                       const int* restrict src2, const int* restrict val1,
                       const int* restrict val2, int n) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        /* Inner conditional with LE_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        dst[i] = (temp <= val1[i]) ? temp : val2[i];
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][16], const int src1[][16], 
                    const int src2[][16], int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 16; j++) {
            /* GT_EXPR for rows, LE_EXPR for columns */
            dst[i][j] = (i > rows/2) ? 
                       ((src1[i][j] <= src2[i][j]) ? src1[i][j] : src2[i][j]) :
                       ((src1[i][j] > src2[i][j]) ? src2[i][j] : src1[i][j]);
        }
    }
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int* restrict dst, const int* restrict src1,
                   const int* restrict src2, const int* restrict val1,
                   const int* restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with GCC unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int* restrict dst, const int* restrict src1,
                                const int* restrict src2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? src1[i] : src2[i];
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ i;
    }
    return sum;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* ALIGNED src1 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED src2 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val1 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val2 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst1 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst2 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst3 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst4 = (int*)aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst5 = (int*)aligned_alloc(32, N * sizeof(int));
    
    /* Multi-dimensional arrays */
    int (*ALIGNED md_dst)[16] = (int(*)[16])aligned_alloc(32, 64 * 16 * sizeof(int));
    int (*ALIGNED md_src1)[16] = (int(*)[16])aligned_alloc(32, 64 * 16 * sizeof(int));
    int (*ALIGNED md_src2)[16] = (int(*)[16])aligned_alloc(32, 64 * 16 * sizeof(int));
    
    init_arrays(src1, src2, val1, val2, N);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            md_src1[i][j] = (i * 16 + j) % 19;
            md_src2[i][j] = (i * 16 + j) % 11;
        }
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
    
    /* Test mixed/nested patterns */
    test_mixed_nested(dst5, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst5, N);
    
    /* Test multi-dimensional */
    test_multi_dim(md_dst, md_src1, md_src2, 64);
    for (int i = 0; i < 64; i++) {
        checksum += compute_checksum(md_dst[i], 16);
    }
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst1, src1, src2, val1, val2, N);
    checksum += compute_checksum(dst1, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst2, src1, src2, N);
    checksum += compute_checksum(dst2, N);
    
    /* Use volatile sink to prevent optimization */
    sink = checksum;
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(src1); free(src2); free(val1); free(val2);
    free(dst1); free(dst2); free(dst3); free(dst4); free(dst5);
    free(md_dst); free(md_src1); free(md_src2);
    
    return 0;
}
