#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
void init_arrays(int* ALIGNED src1, int* ALIGNED src2, 
                 int* ALIGNED val1, int* ALIGNED val2, int size) {
    for (int i = 0; i < size; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) */
__attribute__((noinline, pure))
int test_gt(int* restrict dst, const int* restrict src1, 
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
int test_ge(int* restrict dst, const int* restrict src1,
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Test LT_EXPR (<) */
__attribute__((noinline, pure))
int test_lt(int* restrict dst, const int* restrict src1,
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Test LE_EXPR (<=) */
__attribute__((noinline, pure))
int test_le(int* restrict dst, const int* restrict src1,
            const int* restrict src2, const int* restrict val1,
            const int* restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    return sum;
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
int test_mixed_comparisons(int* restrict dst1, int* restrict dst2,
                           const int* restrict src1, const int* restrict src2,
                           const int* restrict val1, const int* restrict val2,
                           int n) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        sum += dst1[i];
    }
    
    /* Inner loop with LE_EXPR - different comparison type */
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst2[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        sum += dst2[i];
    }
    
    return sum;
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
int test_omp_simd(int* restrict dst, const int* restrict src1,
                  const int* restrict src2, const int* restrict val1,
                  const int* restrict val2, int n) {
    int sum = 0;
    
    #pragma omp simd reduction(+:sum)
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        sum += dst[i];
    }
    
    return sum;
}

/* Multi-dimensional array with different comparison types per dimension */
__attribute__((noinline))
int test_multi_dim(int dst[][N], const int src1[][N], const int src2[][N],
                   const int val1[][N], const int val2[][N], int rows) {
    int sum = 0;
    
    for (int i = 0; i < rows; i++) {
        /* First dimension uses GT_EXPR */
        for (int j = 0; j < N; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
            sum += dst[i][j];
        }
        
        /* Second pass on same dimension uses LE_EXPR */
        for (int j = 0; j < N; j++) {
            dst[i][j] = (src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j];
            sum += dst[i][j];
        }
    }
    
    return sum;
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* ALIGNED src1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED src2 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val2 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst2 = aligned_alloc(32, N * sizeof(int));
    
    /* Multi-dimensional arrays */
    int (*ALIGNED md_dst)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_src1)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_src2)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_val1)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_val2)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    
    if (!src1 || !src2 || !val1 || !val2 || !dst1 || !dst2 ||
        !md_dst || !md_src1 || !md_src2 || !md_val1 || !md_val2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(src1, src2, val1, val2, N);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < N; j++) {
            md_src1[i][j] = (i * N + j) % 13 * 3;
            md_src2[i][j] = (i * N + j) % 7 * 5;
            md_val1[i][j] = (i * N + j) * 2;
            md_val2[i][j] = (i * N + j) * 3;
        }
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator individually */
    total_checksum += test_gt(dst1, src1, src2, val1, val2, N);
    total_checksum += test_ge(dst2, src1, src2, val1, val2, N);
    total_checksum += test_lt(dst1, src1, src2, val1, val2, N);
    total_checksum += test_le(dst2, src1, src2, val1, val2, N);
    
    /* Test mixed comparisons */
    total_checksum += test_mixed_comparisons(dst1, dst2, src1, src2, val1, val2, N);
    
    /* Test with OpenMP SIMD */
    total_checksum += test_omp_simd(dst1, src1, src2, val1, val2, N);
    
    /* Test multi-dimensional with different comparison types */
    total_checksum += test_multi_dim(md_dst, md_src1, md_src2, md_val1, md_val2, 4);
    
    /* Prevent dead code elimination */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Free allocated memory */
    free(src1); free(src2); free(val1); free(val2);
    free(dst1); free(dst2);
    free(md_dst); free(md_src1); free(md_src2);
    free(md_val1); free(md_val2);
    
    return 0;
}
