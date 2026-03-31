#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Helper to prevent dead code elimination
static volatile int sink = 0;

// Test functions for each comparison operator
__attribute__((noinline, pure))
void test_gt(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        // GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline, pure))
void test_ge(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        // GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline, pure))
void test_lt(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        // LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR with swap
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

__attribute__((noinline, pure))
void test_le(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; ++i) {
        // LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR with swap
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

// Mixed comparison types in nested context
__attribute__((noinline))
void test_mixed(int* restrict dst, const int* restrict src1, const int* restrict src2,
                const int* restrict val1, const int* restrict val2) {
    // Outer loop with GT_EXPR
    for (int i = 0; i < N/2; ++i) {
        // Inner loop with LE_EXPR - stresses pattern matching
        for (int j = 0; j < 2; ++j) {
            int idx = i*2 + j;
            // Mix GT and LE comparisons
            if (src1[idx] > src2[idx]) {
                dst[idx] = val1[idx];
            } else if (src1[idx] <= src2[idx]) {
                dst[idx] = val2[idx];
            }
        }
    }
}

// Multi-dimensional array with different comparison types
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8],
                    const int val1[][8], const int val2[][8]) {
    const int rows = N/8;
    
    // First dimension uses GT_EXPR
    for (int i = 0; i < rows; ++i) {
        // Second dimension uses LE_EXPR
        for (int j = 0; j < 8; ++j) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
        }
    }
}

// Test with OpenMP SIMD pragma
__attribute__((noinline))
void test_omp_simd(int* restrict dst, const int* restrict src1, const int* restrict src2,
                   const int* restrict val1, const int* restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; ++i) {
        // GE_EXPR with explicit SIMD vectorization
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

// Test with unroll pragma before vectorization
__attribute__((noinline))
void test_unroll_vectorize(int* restrict dst, const int* restrict src1, const int* restrict src2,
                           const int* restrict val1, const int* restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; ++i) {
        // LT_EXPR with unrolled loop
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

// Compute checksum to prevent dead code elimination
int checksum(const int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    // Aligned arrays for better vectorization
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst[N];
    ALIGNED int dst2[N/8][8], src1_2d[N/8][8], src2_2d[N/8][8];
    ALIGNED int val1_2d[N/8][8], val2_2d[N/8][8];
    
    // Initialize with varying patterns to create mix of true/false conditions
    for (int i = 0; i < N; ++i) {
        src1[i] = (i * 7) % 31;
        src2[i] = (i * 3) % 31;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
    
    for (int i = 0; i < N/8; ++i) {
        for (int j = 0; j < 8; ++j) {
            int idx = i*8 + j;
            src1_2d[i][j] = (idx * 11) % 29;
            src2_2d[i][j] = (idx * 5) % 29;
            val1_2d[i][j] = idx * 4;
            val2_2d[i][j] = idx * 5;
        }
    }
    
    int total_checksum = 0;
    
    // Test each comparison operator
    memset(dst, 0, sizeof(dst));
    test_gt(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    memset(dst, 0, sizeof(dst));
    test_ge(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    memset(dst, 0, sizeof(dst));
    test_lt(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    memset(dst, 0, sizeof(dst));
    test_le(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    // Test mixed comparisons
    memset(dst, 0, sizeof(dst));
    test_mixed(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    // Test multi-dimensional
    memset(dst2, 0, sizeof(dst2));
    test_multi_dim(dst2, src1_2d, src2_2d, val1_2d, val2_2d);
    for (int i = 0; i < N/8; ++i) {
        total_checksum += checksum(dst2[i], 8);
    }
    
    // Test with OpenMP SIMD
    memset(dst, 0, sizeof(dst));
    test_omp_simd(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    // Test with unroll pragma
    memset(dst, 0, sizeof(dst));
    test_unroll_vectorize(dst, src1, src2, val1, val2);
    total_checksum += checksum(dst, N);
    
    sink = total_checksum;
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
