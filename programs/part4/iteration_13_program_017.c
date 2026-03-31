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

/* Test GT_EXPR (>) */
__attribute__((noinline))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    #pragma GCC unroll 4
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) */
__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    #pragma omp simd
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) */
__attribute__((noinline))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) */
__attribute__((noinline))
void test_le(int *restrict dst, const int *restrict src1,
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
                           int n, int m) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < n; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < m; j++) {
            int idx = i * m + j;
            if (idx < N) {
                dst2[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
            }
        }
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][16], const int src1[][16], 
                    const int src2[][16], int rows) {
    for (int i = 0; i < rows; i++) {
        /* First dimension uses GT_EXPR */
        for (int j = 0; j < 8; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? 1 : 0;
        }
        /* Second half uses LE_EXPR */
        for (int j = 8; j < 16; j++) {
            dst[i][j] = (src1[i][j] <= src2[i][j]) ? 1 : 0;
        }
    }
}

/* Complex conditional with multiple comparisons */
__attribute__((noinline))
void test_complex_condition(int *restrict dst, const int *restrict src1,
                           const int *restrict src2, const int *restrict src3,
                           int n) {
    for (int i = 0; i < n; i++) {
        /* Combined condition with different comparison operators */
        dst[i] = (src1[i] > src2[i]) ? 
                 ((src2[i] < src3[i]) ? src1[i] : src3[i]) :
                 ((src1[i] >= src3[i]) ? src2[i] : src3[i]);
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N];
    ALIGNED int dst_mixed1[N], dst_mixed2[N];
    ALIGNED int multi_dst[64][16], multi_src1[64][16], multi_src2[64][16];
    ALIGNED int src3[N], dst_complex[N];
    
    int total_checksum = 0;
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2, N);
    
    /* Initialize additional arrays with different patterns */
    for (int i = 0; i < N; i++) {
        src3[i] = (i % 11) * 7;
    }
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            multi_src1[i][j] = i * 16 + j;
            multi_src2[i][j] = (i * 16 + j) % 100;
        }
    }
    
    /* Test each comparison operator individually */
    test_gt(dst1, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2, N);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed1, dst_mixed2, src1, src2, val1, val2, N/2, 2);
    total_checksum += compute_checksum(dst_mixed1, N/2);
    total_checksum += compute_checksum(dst_mixed2, N);
    
    /* Test multi-dimensional arrays */
    test_multi_dim(multi_dst, multi_src1, multi_src2, 64);
    for (int i = 0; i < 64; i++) {
        total_checksum += compute_checksum(multi_dst[i], 16);
    }
    
    /* Test complex conditional */
    test_complex_condition(dst_complex, src1, src2, src3, N);
    total_checksum += compute_checksum(dst_complex, N);
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
