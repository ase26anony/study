#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR case */
__attribute__((noinline))
void test_gt(int* restrict dst, const int* restrict src1, 
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR case */
__attribute__((noinline))
void test_ge(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR case */
__attribute__((noinline))
void test_lt(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR case */
__attribute__((noinline))
void test_le(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed(int* restrict dst, const int* restrict src1,
                const int* restrict src2, const int* restrict val1,
                const int* restrict val2, int n, int m) {
    for (int i = 0; i < n; ++i) {
        /* Outer loop with GT_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner loop with LE_EXPR - may cause multiple passes */
        #pragma omp simd
        for (int j = 0; j < m; ++j) {
            int idx = i * m + j;
            dst[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
        }
        
        dst[i] += temp;  /* Combine results */
    }
}

/* Test with unroll pragma */
__attribute__((noinline))
void test_unrolled(int* restrict dst, const int* restrict src1,
                   const int* restrict src2, const int* restrict val1,
                   const int* restrict val2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unsigned types to ensure integer comparisons */
__attribute__((noinline))
void test_unsigned(unsigned int* restrict dst, 
                   const unsigned int* restrict src1,
                   const unsigned int* restrict src2,
                   const unsigned int* restrict val1,
                   const unsigned int* restrict val2, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Multi-dimensional test with different comparisons per dimension */
__attribute__((noinline))
void test_2d(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        /* First dimension uses GE_EXPR */
        int row_cond = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        
        for (int j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            /* Second dimension uses LT_EXPR */
            dst[idx] = (src1[idx] < src2[idx]) ? val1[idx] : val2[idx];
            dst[idx] += row_cond;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
__attribute__((pure))
int compute_checksum(const int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Allocate and initialize arrays */
    int* src1 = (int*)aligned_alloc(64, N * sizeof(int));
    int* src2 = (int*)aligned_alloc(64, N * sizeof(int));
    int* val1 = (int*)aligned_alloc(64, N * sizeof(int));
    int* val2 = (int*)aligned_alloc(64, N * sizeof(int));
    int* dst = (int*)aligned_alloc(64, N * sizeof(int));
    
    unsigned int* usrc1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int* usrc2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int* uval1 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int* uval2 = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    unsigned int* udst = (unsigned int*)aligned_alloc(64, N * sizeof(unsigned int));
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = i % 7;
        src2[i] = (i % 5) + 2;
        val1[i] = i * 3;
        val2[i] = i * 5;
        
        usrc1[i] = (i % 11) * 2;
        usrc2[i] = (i % 13) * 3;
        uval1[i] = i * 7;
        uval2[i] = i * 11;
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator */
    memset(dst, 0, N * sizeof(int));
    test_gt(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    memset(dst, 0, N * sizeof(int));
    test_ge(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    memset(dst, 0, N * sizeof(int));
    test_lt(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    memset(dst, 0, N * sizeof(int));
    test_le(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    /* Test mixed comparisons */
    int* dst2 = (int*)aligned_alloc(64, N * M * sizeof(int));
    int* src1_2d = (int*)aligned_alloc(64, N * M * sizeof(int));
    int* src2_2d = (int*)aligned_alloc(64, N * M * sizeof(int));
    int* val1_2d = (int*)aligned_alloc(64, N * M * sizeof(int));
    int* val2_2d = (int*)aligned_alloc(64, N * M * sizeof(int));
    
    for (int i = 0; i < N * M; ++i) {
        src1_2d[i] = i % 17;
        src2_2d[i] = (i % 19) + 3;
        val1_2d[i] = i * 2;
        val2_2d[i] = i * 3;
    }
    
    memset(dst2, 0, N * M * sizeof(int));
    test_mixed(dst2, src1_2d, src2_2d, val1_2d, val2_2d, N, M);
    total_checksum += compute_checksum(dst2, N * M);
    
    /* Test with unroll pragma */
    memset(dst, 0, N * sizeof(int));
    test_unrolled(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    /* Test unsigned comparisons */
    memset(udst, 0, N * sizeof(unsigned int));
    test_unsigned(udst, usrc1, usrc2, uval1, uval2, N);
    for (int i = 0; i < N; ++i) {
        total_checksum += (int)udst[i];
    }
    
    /* Test 2D with different comparisons per dimension */
    memset(dst2, 0, N * M * sizeof(int));
    test_2d(dst2, src1_2d, src2_2d, val1_2d, val2_2d, 64, 16);
    total_checksum += compute_checksum(dst2, 64 * 16);
    
    /* Prevent compiler from optimizing everything away */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(val1);
    free(val2);
    free(dst);
    free(usrc1);
    free(usrc2);
    free(uval1);
    free(uval2);
    free(udst);
    free(dst2);
    free(src1_2d);
    free(src2_2d);
    free(val1_2d);
    free(val2_2d);
    
    return 0;
}
