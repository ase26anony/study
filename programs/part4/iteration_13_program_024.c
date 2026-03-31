#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns to create mixed conditions */
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
void test_gt(int* ALIGNED restrict dst, 
             const int* ALIGNED restrict src1,
             const int* ALIGNED restrict src2,
             const int* ALIGNED restrict val1,
             const int* ALIGNED restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
void test_ge(int* ALIGNED restrict dst,
             const int* ALIGNED restrict src1,
             const int* ALIGNED restrict src2,
             const int* ALIGNED restrict val1,
             const int* ALIGNED restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) */
__attribute__((noinline, pure))
void test_lt(int* ALIGNED restrict dst,
             const int* ALIGNED restrict src1,
             const int* ALIGNED restrict src2,
             const int* ALIGNED restrict val1,
             const int* ALIGNED restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) */
__attribute__((noinline, pure))
void test_le(int* ALIGNED restrict dst,
             const int* ALIGNED restrict src1,
             const int* ALIGNED restrict src2,
             const int* ALIGNED restrict val1,
             const int* ALIGNED restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int* ALIGNED restrict dst1,
                            int* ALIGNED restrict dst2,
                            const int* ALIGNED restrict src1,
                            const int* ALIGNED restrict src2,
                            const int* ALIGNED restrict val1,
                            const int* ALIGNED restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner conditional with LE_EXPR - stresses pattern matching */
        dst2[i] = (src1[i] <= src2[i]) ? val2[i] : val1[i];
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][N],
                    const int src1[][N],
                    const int src2[][N],
                    const int val1[][N],
                    const int val2[][N],
                    int rows) {
    for (int r = 0; r < rows; r++) {
        /* Row processing uses GT_EXPR */
        for (int c = 0; c < N; c++) {
            dst[r][c] = (src1[r][c] > src2[r][c]) ? val1[r][c] : val2[r][c];
        }
        
        /* Column processing uses LE_EXPR */
        for (int c = 0; c < N; c++) {
            dst[r][c] += (src1[r][c] <= src2[r][c]) ? val2[r][c] : val1[r][c];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(const int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
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
    
    /* Multi-dimensional arrays */
    int (*ALIGNED md_dst)[N] = (int(*)[N])aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_src1)[N] = (int(*)[N])aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_src2)[N] = (int(*)[N])aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_val1)[N] = (int(*)[N])aligned_alloc(32, 4 * N * sizeof(int));
    int (*ALIGNED md_val2)[N] = (int(*)[N])aligned_alloc(32, 4 * N * sizeof(int));
    
    if (!src1 || !src2 || !val1 || !val2 || !dst1 || !dst2 ||
        !md_dst || !md_src1 || !md_src2 || !md_val1 || !md_val2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(src1, src2, val1, val2, N);
    
    /* Initialize multi-dimensional arrays */
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < N; c++) {
            md_src1[r][c] = (r * N + c) % 17;
            md_src2[r][c] = (r * N + c) % 11;
            md_val1[r][c] = (r * N + c) * 2;
            md_val2[r][c] = (r * N + c) * 3;
        }
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += checksum(dst2, N);
    
    test_lt(dst1, src1, src2, val1, val2);
    total_checksum += checksum(dst1, N);
    
    test_le(dst2, src1, src2, val1, val2);
    total_checksum += checksum(dst2, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst1, dst2, src1, src2, val1, val2);
    total_checksum += checksum(dst1, N);
    total_checksum += checksum(dst2, N);
    
    /* Test multi-dimensional with different comparison types */
    test_multi_dim(md_dst, md_src1, md_src2, md_val1, md_val2, 4);
    for (int r = 0; r < 4; r++) {
        total_checksum += checksum(md_dst[r], N);
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(val1);
    free(val2);
    free(dst1);
    free(dst2);
    free(md_dst);
    free(md_src1);
    free(md_src2);
    free(md_val1);
    free(md_val2);
    
    return 0;
}
