#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper functions with no side effects to aid vectorization */
__attribute__((const)) static int pure_gt(int a, int b) { return a > b; }
__attribute__((const)) static int pure_ge(int a, int b) { return a >= b; }
__attribute__((const)) static int pure_lt(int a, int b) { return a < b; }
__attribute__((const)) static int pure_le(int a, int b) { return a <= b; }

/* Test functions for each comparison operator */

/* GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR */
__attribute__((noinline))
void test_gt(int* restrict dst, const int* restrict src1, 
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR */
__attribute__((noinline))
void test_ge(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap */
__attribute__((noinline))
void test_lt(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
__attribute__((noinline))
void test_le(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_nested(int* restrict dst, const int* restrict src1,
                       const int* restrict src2, const int* restrict val1,
                       const int* restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; ++i) {
        /* Inner loop with LE_EXPR - should trigger swap logic */
        #pragma GCC unroll 4
        for (int j = 0; j < 2; ++j) {
            int idx = i*2 + j;
            dst[idx] = (src1[idx] > src2[idx]) ? 
                      ((src1[idx] <= val1[idx]) ? val1[idx] : val2[idx]) : 
                      val2[idx];
        }
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int* restrict dst, const int* restrict src1,
                    const int* restrict src2, const int* restrict val1,
                    const int* restrict val2) {
    const int ROWS = 32;
    const int COLS = 32;
    
    /* First dimension uses GT_EXPR */
    for (int i = 0; i < ROWS; ++i) {
        /* Second dimension uses LE_EXPR - should trigger swap */
        #pragma omp simd
        for (int j = 0; j < COLS; ++j) {
            int idx = i * COLS + j;
            dst[idx] = (i > j) ? 
                      ((src1[idx] <= src2[idx]) ? val1[idx] : val2[idx]) : 
                      val2[idx];
        }
    }
}

/* Complex pattern with multiple conditions */
__attribute__((noinline))
void test_complex_pattern(int* restrict dst, const int* restrict src1,
                          const int* restrict src2, const int* restrict val1,
                          const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        /* Mix of GE_EXPR and LT_EXPR in same expression */
        int cond1 = pure_ge(src1[i], src2[i]);
        int cond2 = pure_lt(val1[i], val2[i]);
        dst[i] = (cond1 && cond2) ? val1[i] : 
                ((src1[i] <= val1[i]) ? src2[i] : val2[i]);
    }
}

/* Compute checksum to prevent dead code elimination */
__attribute__((noinline))
unsigned long long compute_checksum(const int* arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += (unsigned long long)arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    ALIGNED int dst_mixed[N], dst_multi[N], dst_complex[N];
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i * 7) % 31;
        src2[i] = (i * 3) % 31;
        val1[i] = (i * 5) % 17;
        val2[i] = (i * 11) % 17;
    }
    
    unsigned long long total_checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst_gt, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_gt, N);
    
    test_ge(dst_ge, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_ge, N);
    
    test_lt(dst_lt, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_lt, N);
    
    test_le(dst_le, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_le, N);
    
    /* Test nested/mixed patterns */
    test_mixed_nested(dst_mixed, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_mixed, N);
    
    test_multi_dim(dst_multi, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_multi, 32*32);
    
    test_complex_pattern(dst_complex, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_complex, N);
    
    printf("Total checksum: %llu\n", total_checksum);
    
    /* Verify with a simple test case */
    int test_src1[] = {5, 3, 8, 2};
    int test_src2[] = {3, 5, 2, 8};
    int test_val1[] = {100, 200, 300, 400};
    int test_val2[] = {10, 20, 30, 40};
    int test_dst[4];
    
    test_gt(test_dst, test_src1, test_src2, test_val1, test_val2);
    printf("GT test: %d %d %d %d\n", test_dst[0], test_dst[1], test_dst[2], test_dst[3]);
    
    return 0;
}
