#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
void init_arrays(int *restrict src1, int *restrict src2, 
                 int *restrict val1, int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) pattern */
__attribute__((noinline))
void test_gt_expr(int *restrict dst, const int *restrict src1, 
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) pattern */
__attribute__((noinline))
void test_ge_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) pattern */
__attribute__((noinline))
void test_lt_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) pattern */
__attribute__((noinline))
void test_le_expr(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst1, int *restrict dst2,
                            const int *restrict src1, const int *restrict src2,
                            const int *restrict val1, const int *restrict val2,
                            int n) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < N) {
                dst2[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
            }
        }
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int *restrict dst, const int *restrict src1,
                    const int *restrict src2, const int *restrict val1,
                    const int *restrict val2, int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        /* First dimension uses GT_EXPR */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            dst[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
        }
        
        /* Second dimension uses LE_EXPR */
        for (int j = 0; j < cols; j++) {
            int idx = i * cols + j;
            dst[idx] = (src1[idx] <= src2[idx]) ? dst[idx] : val2[idx];
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i] ^ (i & 0xFF);
    }
    return sum;
}

int main(void) {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N], dst6[N];
    
    int final_checksum = 0;
    
    /* Initialize source arrays */
    init_arrays(src1, src2, val1, val2, N);
    
    /* Test each comparison operator separately */
    test_gt_expr(dst1, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst1, N);
    
    test_ge_expr(dst2, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst2, N);
    
    test_lt_expr(dst3, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst3, N);
    
    test_le_expr(dst4, src1, src2, val1, val2, N);
    final_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparison patterns */
    test_mixed_comparisons(dst5, dst6, src1, src2, val1, val2, N/8);
    final_checksum += compute_checksum(dst5, N);
    final_checksum += compute_checksum(dst6, N);
    
    /* Test multi-dimensional case */
    test_multi_dim(dst1, src1, src2, val1, val2, 32, 32);
    final_checksum += compute_checksum(dst1, N);
    
    /* Use volatile sink to prevent optimization */
    sink = final_checksum;
    
    printf("Final checksum: %d\n", final_checksum);
    return 0;
}
