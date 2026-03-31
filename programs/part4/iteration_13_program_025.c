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
    #pragma omp simd
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
    #pragma GCC unroll 4
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
int test_mixed_comparisons(int* ALIGNED restrict dst,
                           const int* ALIGNED restrict src1,
                           const int* ALIGNED restrict src2,
                           const int* ALIGNED restrict val1,
                           const int* ALIGNED restrict val2, int n) {
    int sum = 0;
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        /* Inner conditional with LE_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        dst[i] = (temp <= val2[i]) ? temp : val1[i];
    }
    for (int i = 0; i < n; i++) sum += dst[i];
    return sum;
}

/* Multi-dimensional array with different comparisons */
__attribute__((noinline))
int test_multi_dim(int dst[][N], const int src1[][N], 
                   const int src2[][N], int rows, int cols) {
    int sum = 0;
    /* GT_EXPR for rows, LE_EXPR for columns */
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (i > 0) {  /* GT_EXPR context */
                dst[i][j] = (src1[i][j] > src2[i][j]) ? i : j;
            } else {      /* LE_EXPR context */
                dst[i][j] = (src1[i][j] <= src2[i][j]) ? i : j;
            }
        }
    }
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            sum += dst[i][j];
        }
    }
    return sum;
}

/* Test with unsigned types to ensure integer comparisons */
__attribute__((noinline, pure))
int test_unsigned(unsigned int* ALIGNED restrict dst,
                  const unsigned int* ALIGNED restrict src1,
                  const unsigned int* ALIGNED restrict src2,
                  const unsigned int* ALIGNED restrict val1,
                  const unsigned int* ALIGNED restrict val2, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
    for (int i = 0; i < n; i++) sum += dst[i];
    return (int)sum;
}

int main() {
    /* Allocate aligned arrays */
    int* ALIGNED src1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED src2 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val1 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED val2 = aligned_alloc(32, N * sizeof(int));
    int* ALIGNED dst = aligned_alloc(32, N * sizeof(int));
    
    /* For multi-dimensional test */
    int (*dst_2d)[N] = aligned_alloc(32, N * N * sizeof(int));
    int (*src1_2d)[N] = aligned_alloc(32, N * N * sizeof(int));
    int (*src2_2d)[N] = aligned_alloc(32, N * N * sizeof(int));
    
    /* For unsigned test */
    unsigned int* ALIGNED usrc1 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED usrc2 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED uval1 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED uval2 = aligned_alloc(32, N * sizeof(unsigned int));
    unsigned int* ALIGNED udst = aligned_alloc(32, N * sizeof(unsigned int));
    
    if (!src1 || !src2 || !val1 || !val2 || !dst || 
        !dst_2d || !src1_2d || !src2_2d ||
        !usrc1 || !usrc2 || !uval1 || !uval2 || !udst) {
        fprintf(stderr, "Allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(src1, src2, val1, val2, N);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            src1_2d[i][j] = (i * j) % 17;
            src2_2d[i][j] = (i + j) % 13;
        }
    }
    
    /* Initialize unsigned arrays */
    for (int i = 0; i < N; i++) {
        usrc1[i] = (i % 11) * 7;
        usrc2[i] = (i % 5) * 11;
        uval1[i] = i * 4;
        uval2[i] = i * 5;
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator */
    total_checksum += test_gt(dst, src1, src2, val1, val2, N);
    total_checksum += test_ge(dst, src1, src2, val1, val2, N);
    total_checksum += test_lt(dst, src1, src2, val1, val2, N);
    total_checksum += test_le(dst, src1, src2, val1, val2, N);
    
    /* Test mixed comparisons */
    total_checksum += test_mixed_comparisons(dst, src1, src2, val1, val2, N);
    
    /* Test multi-dimensional */
    total_checksum += test_multi_dim(dst_2d, src1_2d, src2_2d, N/16, N/16);
    
    /* Test unsigned */
    total_checksum += test_unsigned(udst, usrc1, usrc2, uval1, uval2, N);
    
    /* Use result to prevent elimination */
    sink = total_checksum;
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(src1); free(src2); free(val1); free(val2); free(dst);
    free(dst_2d); free(src1_2d); free(src2_2d);
    free(usrc1); free(usrc2); free(uval1); free(uval2); free(udst);
    
    return 0;
}
