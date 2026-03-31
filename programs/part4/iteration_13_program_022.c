#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns to create mix of true/false conditions */
void init_arrays(int* restrict src1, int* restrict src2, 
                 int* restrict val1, int* restrict val2) {
    for (int i = 0; i < N; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) - maps to BIT_NOT_EXPR, BIT_AND_EXPR */
__attribute__((noinline, pure))
int test_gt_expr(int* restrict dst, const int* restrict src1, 
                 const int* restrict src2, const int* restrict val1,
                 const int* restrict val2) {
    int sum = 0;
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
    
    /* Checksum to prevent elimination */
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test GE_EXPR (>=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR */
__attribute__((noinline, pure))
int test_ge_expr(int* restrict dst, const int* restrict src1,
                 const int* restrict src2, const int* restrict val1,
                 const int* restrict val2) {
    int sum = 0;
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
    
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test LT_EXPR (<) - maps to BIT_NOT_EXPR, BIT_AND_EXPR with swap */
__attribute__((noinline, pure))
int test_lt_expr(int* restrict dst, const int* restrict src1,
                 const int* restrict src2, const int* restrict val1,
                 const int* restrict val2) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
    
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test LE_EXPR (<=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
__attribute__((noinline, pure))
int test_le_expr(int* restrict dst, const int* restrict src1,
                 const int* restrict src2, const int* restrict val1,
                 const int* restrict val2) {
    int sum = 0;
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
    
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Mixed comparison types in nested context to stress pattern matching */
__attribute__((noinline))
int test_mixed_comparisons(int* restrict dst1, int* restrict dst2,
                          const int* restrict src1, const int* restrict src2,
                          const int* restrict val1, const int* restrict val2) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; i++) {
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < N) {
                /* GT_EXPR in outer context */
                dst1[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
                
                /* LE_EXPR in inner context - may cause multiple passes */
                dst2[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        sum += dst1[i] + dst2[i];
    }
    return sum;
}

/* Multi-dimensional array processing with different comparison types */
__attribute__((noinline))
int test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8],
                   const int val1[][8], const int val2[][8]) {
    int sum = 0;
    const int rows = N/8;
    
    /* First dimension uses GT_EXPR */
    for (int i = 0; i < rows; i++) {
        /* Second dimension uses LE_EXPR */
        for (int j = 0; j < 8; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? val1[i][j] : val2[i][j];
            /* Additional conditional with different operator */
            if (src1[i][j] <= src2[i][j]) {
                dst[i][j] += 1;  /* Simple modification */
            }
        }
    }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 8; j++) {
            sum += dst[i][j];
        }
    }
    return sum;
}

int main() {
    /* Use aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N];
    ALIGNED int dst_mixed1[N], dst_mixed2[N];
    ALIGNED int dst_multi[N/8][8];
    ALIGNED int src1_multi[N/8][8], src2_multi[N/8][8];
    ALIGNED int val1_multi[N/8][8], val2_multi[N/8][8];
    
    int final_checksum = 0;
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            src1_multi[i][j] = src1[idx];
            src2_multi[i][j] = src2[idx];
            val1_multi[i][j] = val1[idx];
            val2_multi[i][j] = val2[idx];
        }
    }
    
    /* Test each comparison operator in vectorizable loops */
    final_checksum += test_gt_expr(dst1, src1, src2, val1, val2);
    final_checksum += test_ge_expr(dst2, src1, src2, val1, val2);
    final_checksum += test_lt_expr(dst3, src1, src2, val1, val2);
    final_checksum += test_le_expr(dst4, src1, src2, val1, val2);
    
    /* Test mixed comparisons */
    final_checksum += test_mixed_comparisons(dst_mixed1, dst_mixed2, 
                                            src1, src2, val1, val2);
    
    /* Test multi-dimensional with different operators */
    final_checksum += test_multi_dim(dst_multi, src1_multi, src2_multi,
                                    val1_multi, val2_multi);
    
    /* Use volatile sink to prevent optimization */
    sink = final_checksum;
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
