#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

// Helper to prevent dead code elimination
static volatile int sink = 0;

// Pure function attribute to aid vectorizer analysis
__attribute__((pure))
int compute_checksum(const int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

// Test GT_EXPR (>) - maps to BIT_NOT_EXPR, BIT_AND_EXPR
__attribute__((noinline))
void test_gt_expr(int* restrict dst, const int* restrict src1, 
                  const int* restrict src2, const int* restrict val1,
                  const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        // GT_EXPR pattern: src1[i] > src2[i]
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

// Test GE_EXPR (>=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR  
__attribute__((noinline))
void test_ge_expr(int* restrict dst, const int* restrict src1,
                  const int* restrict src2, const int* restrict val1,
                  const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        // GE_EXPR pattern: src1[i] >= src2[i]
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

// Test LT_EXPR (<) - maps to BIT_NOT_EXPR, BIT_AND_EXPR with swap
__attribute__((noinline))
void test_lt_expr(int* restrict dst, const int* restrict src1,
                  const int* restrict src2, const int* restrict val1,
                  const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        // LT_EXPR pattern: src1[i] < src2[i]
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

// Test LE_EXPR (<=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR with swap
__attribute__((noinline))
void test_le_expr(int* restrict dst, const int* restrict src1,
                  const int* restrict src2, const int* restrict val1,
                  const int* restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        // LE_EXPR pattern: src1[i] <= src2[i]
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

// Mixed comparison types in nested context
__attribute__((noinline))
void test_mixed_comparisons(int* restrict dst, const int* restrict src1,
                           const int* restrict src2, const int* restrict val1,
                           const int* restrict val2, int n) {
    // Outer loop with GT_EXPR
    for (int i = 0; i < n; i++) {
        // Inner conditional with LE_EXPR - should trigger swap logic
        if (src1[i] > src2[i]) {
            dst[i] = (src1[i] <= val1[i]) ? val1[i] : val2[i];
        } else {
            dst[i] = (src2[i] < val2[i]) ? val2[i] : val1[i];
        }
    }
}

// Test with #pragma omp simd for explicit vectorization
__attribute__((noinline))
void test_omp_simd(int* restrict dst, const int* restrict src1,
                   const int* restrict src2, const int* restrict val1,
                   const int* restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        // Mix of comparison operators
        dst[i] = (src1[i] > src2[i]) ? 
                 ((val1[i] >= val2[i]) ? val1[i] : val2[i]) :
                 ((val1[i] <= val2[i]) ? val1[i] : val2[i]);
    }
}

// Test with unroll pragma before vectorization
__attribute__((noinline))
void test_unroll_then_vectorize(int* restrict dst, const int* restrict src1,
                               const int* restrict src2, const int* restrict val1,
                               const int* restrict val2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        // GE_EXPR pattern
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

// Multi-dimensional array processing with different comparisons
__attribute__((noinline))
void test_multi_dim(int dst[][N], const int src1[][N], const int src2[][N],
                   const int val1[][N], const int val2[][N], int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Different comparisons for different dimensions
            if (i > 0) {  // GT_EXPR for row dimension
                dst[i][j] = (src1[i][j] <= src2[i][j]) ? val1[i][j] : val2[i][j];
            } else {
                dst[i][j] = (src1[i][j] >= src2[i][j]) ? val1[i][j] : val2[i][j];
            }
        }
    }
}

int main() {
    // Aligned arrays for better vectorization
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst[N];
    ALIGNED int dst2[N][N];
    ALIGNED int src1_2d[N][N], src2_2d[N][N], val1_2d[N][N], val2_2d[N][N];
    
    int total_checksum = 0;
    
    // Initialize with varying patterns to create mix of true/false conditions
    for (int i = 0; i < N; i++) {
        src1[i] = (i * 7) % 31;
        src2[i] = (i * 3) % 31;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
    
    // Initialize 2D arrays
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            src1_2d[i][j] = (i * 13 + j * 7) % 31;
            src2_2d[i][j] = (i * 5 + j * 11) % 31;
            val1_2d[i][j] = i * 3 + j * 2;
            val2_2d[i][j] = i * 2 + j * 3;
        }
    }
    
    // Test each comparison operator
    test_gt_expr(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    test_ge_expr(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    test_lt_expr(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    test_le_expr(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    // Test mixed comparisons
    test_mixed_comparisons(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    // Test with OpenMP SIMD
    test_omp_simd(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    // Test with unroll pragma
    test_unroll_then_vectorize(dst, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst, N);
    
    // Test multi-dimensional (smaller size for speed)
    test_multi_dim(dst2, src1_2d, src2_2d, val1_2d, val2_2d, 64, 64);
    for (int i = 0; i < 64; i++) {
        total_checksum += compute_checksum(dst2[i], 64);
    }
    
    // Use volatile sink to prevent optimization
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
