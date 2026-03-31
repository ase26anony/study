#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
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
int test_gt(int* ALIGNED restrict dst, 
            const int* ALIGNED restrict src1,
            const int* ALIGNED restrict src2,
            const int* ALIGNED restrict val1,
            const int* ALIGNED restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
    /* Checksum to prevent elimination */
    for (int i = 0; i < n; i++) sum += dst[i];
    return sum;
}

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
int test_ge(int* ALIGNED restrict dst,
            const int* ALIGNED restrict src1,
            const int* ALIGNED restrict src2,
            const int* ALIGNED restrict val1,
            const int* ALIGNED restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
    for (int i = 0; i < n; i++) sum += dst[i];
    return sum;
}

/* Test LT_EXPR (<) */
__attribute__((noinline, pure))
int test_lt(int* ALIGNED restrict dst,
            const int* ALIGNED restrict src1,
            const int* ALIGNED restrict src2,
            const int* ALIGNED restrict val1,
            const int* ALIGNED restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
    for (int i = 0; i < n; i++) sum += dst[i];
    return sum;
}

/* Test LE_EXPR (<=) */
__attribute__((noinline, pure))
int test_le(int* ALIGNED restrict dst,
            const int* ALIGNED restrict src1,
            const int* ALIGNED restrict src2,
            const int* ALIGNED restrict val1,
            const int* ALIGNED restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
    for (int i = 0; i < n; i++) sum += dst[i];
    return sum;
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
int test_mixed(int* ALIGNED restrict dst1,
               int* ALIGNED restrict dst2,
               const int* ALIGNED restrict src1,
               const int* ALIGNED restrict src2,
               const int* ALIGNED restrict val1,
               const int* ALIGNED restrict val2, int n) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
    
    /* Inner loop with LE_EXPR */
    for (int i = 0; i < n; i++) {
        dst2[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
    
    for (int i = 0; i < n; i++) sum += dst1[i] + dst2[i];
    return sum;
}

/* Multi-dimensional array with different comparisons */
__attribute__((noinline))
int test_multi_dim(int dst[][N],
                   const int src1[][N],
                   const int src2[][N],
                   const int val1[][N],
                   const int val2[][N], int rows) {
    int sum = 0;
    
    /* Row processing with GT_EXPR */
    for (int i = 0; i < rows; i++) {
        #pragma GCC unroll 2
        for (int j = 0; j < N; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
        }
    }
    
    /* Column processing with LE_EXPR */
    for (int j = 0; j < N; j++) {
        #pragma omp simd
        for (int i = 0; i < rows; i++) {
            dst[i][j] += (src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j];
        }
    }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < N; j++) {
            sum += dst[i][j];
        }
    }
    return sum;
}

/* Test with unsigned types */
__attribute__((noinline, pure))
int test_unsigned(unsigned int* ALIGNED restrict dst,
                  const unsigned int* ALIGNED restrict src1,
                  const unsigned int* ALIGNED restrict src2,
                  const unsigned int* ALIGNED restrict val1,
                  const unsigned int* ALIGNED restrict val2, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
    for (int i = 0; i < n; i++) sum += dst[i];
    return sum;
}

int main() {
    /* Allocate aligned arrays */
    int* ALIGNED src1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED src2 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val2 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst2 = aligned_alloc(32, N * sizeof(int));
    
    /* Multi-dimensional arrays */
    int (*md_dst)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*md_src1)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*md_src2)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*md_val1)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    int (*md_val2)[N] = aligned_alloc(32, 4 * N * sizeof(int));
    
    /* Unsigned arrays */
    unsigned int* ALIGNED usrc1 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED usrc2 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED uval1 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED uval2 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED udst = aligned_alloc(32, N * sizeof(unsigned int));
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2, N);
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < N; j++) {
            md_src1[i][j] = (i * N + j) % 17;
            md_src2[i][j] = (i * N + j) % 11;
            md_val1[i][j] = (i * N + j) * 2;
            md_val2[i][j] = (i * N + j) * 3;
        }
    }
    
    for (int i = 0; i < N; i++) {
        usrc1[i] = i * 3;
        usrc2[i] = i * 2;
        uval1[i] = i * 5;
        uval2[i] = i * 7;
    }
    
    int total_sum = 0;
    
    /* Test each comparison operator */
    total_sum += test_gt(dst1, src1, src2, val1, val2, N);
    total_sum += test_ge(dst1, src1, src2, val1, val2, N);
    total_sum += test_lt(dst1, src1, src2, val1, val2, N);
    total_sum += test_le(dst1, src1, src2, val1, val2, N);
    
    /* Test mixed comparisons */
    total_sum += test_mixed(dst1, dst2, src1, src2, val1, val2, N);
    
    /* Test multi-dimensional */
    total_sum += test_multi_dim(md_dst, md_src1, md_src2, md_val1, md_val2, 4);
    
    /* Test unsigned */
    total_sum += test_unsigned(udst, usrc1, usrc2, uval1, uval2, N);
    
    /* Use result to prevent elimination */
    sink = total_sum;
    printf("Total checksum: %d\n", total_sum);
    
    /* Cleanup */
    free(src1); free(src2); free(val1); free(val2);
    free(dst1); free(dst2);
    free(md_dst); free(md_src1); free(md_src2);
    free(md_val1); free(md_val2);
    free(usrc1); free(usrc2); free(uval1); free(uval2);
    free(udst);
    
    return 0;
}
