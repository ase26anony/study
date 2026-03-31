#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
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
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    #pragma omp simd
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
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst1, int *restrict dst2,
                           const int *restrict src1, const int *restrict src2,
                           const int *restrict val1, const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner conditional with LE_EXPR */
        if (i % 2 == 0) {
            dst2[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        } else {
            dst2[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
        }
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8]) {
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            /* GT_EXPR for rows, LE_EXPR for columns */
            if (i > 0) {
                dst[i][j] = (src1[i][j] > src2[i-1][j]) ? 1 : 0;
            } else {
                dst[i][j] = (src1[i][j] <= src2[i][j]) ? 1 : 0;
            }
        }
    }
}

/* Compute checksum to prevent optimization */
int compute_checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main(void) {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    ALIGNED int dst_mixed1[N], dst_mixed2[N];
    ALIGNED int src1_2d[N/8][8], src2_2d[N/8][8], dst_2d[N/8][8];
    
    int total_checksum = 0;
    
    /* Initialize data */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            src1_2d[i][j] = (idx % 13) * 3;
            src2_2d[i][j] = (idx % 7) * 5;
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
    total_checksum += compute_checksum(dst_mixed2, N);
    
    /* Test multi-dimensional case */
    test_multi_dim(dst_2d, src1_2d, src2_2d);
    for (int i = 0; i < N/8; i++) {
        total_checksum += compute_checksum(dst_2d[i], 8);
    }
    
    /* Additional test with unsigned types to ensure pattern matching */
    {
        ALIGNED unsigned int usrc1[N], usrc2[N], udst[N];
        for (int i = 0; i < N; i++) {
            usrc1[i] = i * 2;
            usrc2[i] = i * 3;
        }
        
        #pragma omp simd
        for (int i = 0; i < N; i++) {
            udst[i] = (usrc1[i] >= usrc2[i]) ? usrc1[i] : usrc2[i];
        }
        total_checksum += compute_checksum((int*)udst, N);
    }
    
    /* Prevent dead code elimination */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
