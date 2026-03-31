#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns to create mix of true/false conditions */
void init_arrays(int *restrict src1, int *restrict src2, 
                 int *restrict val1, int *restrict val2) {
    for (int i = 0; i < N; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) */
__attribute__((noinline, pure))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) */
__attribute__((noinline, pure))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) */
__attribute__((noinline, pure))
void test_le(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context - stresses pattern matching */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst1, int *restrict dst2,
                            const int *restrict src1, const int *restrict src2,
                            const int *restrict val1, const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner loop with LE_EXPR - different comparison type */
        if (i < N/2) {
            dst2[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        }
    }
}

/* Test with OpenMP SIMD pragma - different vectorization path */
__attribute__((noinline))
void test_omp_simd(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, const int *restrict val1,
                   const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int *restrict dst, const int *restrict src1,
                                const int *restrict src2, const int *restrict val1,
                                const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Multi-dimensional array with different comparison types per dimension */
__attribute__((noinline))
void test_multi_dim(int dst[][16], const int src1[][16], const int src2[][16]) {
    for (int i = 0; i < N/16; i++) {
        for (int j = 0; j < 16; j++) {
            /* GT_EXPR for first dimension, LE_EXPR for second */
            if (i > 0) {
                dst[i][j] = (src1[i][j] > src2[i][j]) ? 1 : 0;
            } else {
                dst[i][j] = (src1[i][j] <= src2[i][j]) ? 1 : 0;
            }
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    ALIGNED int dst_mixed1[N], dst_mixed2[N];
    ALIGNED int dst_omp[N], dst_unroll[N];
    ALIGNED int dst_multi[N/16][16];
    ALIGNED int src1_multi[N/16][16], src2_multi[N/16][16];
    
    int total_checksum = 0;
    
    /* Initialize source arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/16; i++) {
        for (int j = 0; j < 16; j++) {
            src1_multi[i][j] = (i * 16 + j) % 19;
            src2_multi[i][j] = (i * 16 + j) % 11;
        }
    }
    
    /* Test each comparison operator */
    test_gt(dst_gt, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_gt, N);
    
    test_ge(dst_ge, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_ge, N);
    
    test_lt(dst_lt, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_lt, N);
    
    test_le(dst_le, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_le, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed1, dst_mixed2, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_mixed1, N);
    total_checksum += compute_checksum(dst_mixed2, N/2);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst_omp, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_omp, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst_unroll, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_unroll, N);
    
    /* Test multi-dimensional */
    test_multi_dim(dst_multi, src1_multi, src2_multi);
    for (int i = 0; i < N/16; i++) {
        total_checksum += compute_checksum(dst_multi[i], 16);
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
