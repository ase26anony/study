#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Test functions for each comparison operator */

/* GT_EXPR case */
void test_gt(int* restrict dst, const int* restrict src1, 
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR case */
void test_ge(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR case */
void test_lt(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR case */
void test_le(int* restrict dst, const int* restrict src1,
             const int* restrict src2, const int* restrict val1,
             const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
void test_mixed_nested(int* restrict dst, const int* restrict src1,
                       const int* restrict src2, const int* restrict val1,
                       const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        /* Outer condition uses GT_EXPR */
        int temp = (src1[i] > src2[i]) ? val1[i] : val2[i];
        /* Inner condition uses LE_EXPR */
        dst[i] = (temp <= src1[i]) ? temp : src2[i];
    }
}

/* Multi-dimensional array with different comparison types */
void test_multi_dim(int* restrict dst, const int* restrict src1,
                    const int* restrict src2, const int* restrict val1,
                    const int* restrict val2) {
    const int rows = 32;
    const int cols = 32;
    
    for (int i = 0; i < rows; ++i) {
        #pragma GCC unroll 4
        for (int j = 0; j < cols; ++j) {
            int idx = i * cols + j;
            /* Row condition uses GE_EXPR */
            int row_cond = (i >= j) ? val1[idx] : val2[idx];
            /* Column condition uses LT_EXPR */
            dst[idx] = (src1[idx] < src2[idx]) ? row_cond : src1[idx];
        }
    }
}

/* Test with OpenMP SIMD pragma */
void test_omp_simd(int* restrict dst, const int* restrict src1,
                   const int* restrict src2, const int* restrict val1,
                   const int* restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unsigned types to ensure integer comparisons */
void test_unsigned(unsigned int* restrict dst, const unsigned int* restrict src1,
                   const unsigned int* restrict src2, const unsigned int* restrict val1,
                   const unsigned int* restrict val2) {
    for (unsigned int i = 0; i < N; ++i) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += arr[i] ^ i;
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N];
    ALIGNED int dst_mixed[N], dst_multi[32*32];
    ALIGNED unsigned int usrc1[N], usrc2[N], uval1[N], uval2[N];
    ALIGNED unsigned int udst[N];
    
    /* Initialize with varying patterns to create mix of true/false conditions */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i * 7) % 19;
        src2[i] = (i * 3) % 17;
        val1[i] = i * 2;
        val2[i] = i * 3;
        
        usrc1[i] = (i * 11) % 23;
        usrc2[i] = (i * 5) % 29;
        uval1[i] = i * 4;
        uval2[i] = i * 5;
    }
    
    /* Initialize multi-dimensional array data */
    for (int i = 0; i < 32*32; ++i) {
        src1[i] = (i * 13) % 31;
        src2[i] = (i * 7) % 37;
        val1[i] = i * 6;
        val2[i] = i * 7;
    }
    
    int total_checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed nested comparisons */
    test_mixed_nested(dst_mixed, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_mixed, N);
    
    /* Test multi-dimensional with different comparison types */
    test_multi_dim(dst_multi, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_multi, 32*32);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    /* Test with unsigned types */
    test_unsigned(udst, usrc1, usrc2, uval1, uval2);
    for (int i = 0; i < N; ++i) {
        total_checksum += (int)(udst[i] ^ i);
    }
    
    /* Additional test: Loop with runtime count that's multiple of vector width */
    int dynamic_N = (N / 8) * 8; /* Ensure multiple of typical vector width (8 for ints) */
    for (int i = 0; i < dynamic_N; ++i) {
        dst1[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
    total_checksum += compute_checksum(dst1, dynamic_N);
    
    /* Complex pattern: Multiple conditional assignments in same loop */
    for (int i = 0; i < N; i += 2) {
        /* First uses GT_EXPR */
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        /* Second uses LE_EXPR */
        dst1[i+1] = (src1[i+1] <= src2[i+1]) ? val1[i+1] : val2[i+1];
    }
    total_checksum += compute_checksum(dst1, N);
    
    sink = total_checksum;
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
