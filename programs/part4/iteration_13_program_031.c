#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
void init_arrays(int * restrict a, int * restrict b, int * restrict c, int * restrict d) {
    for (int i = 0; i < N; i++) {
        a[i] = (i % 7) * 3;
        b[i] = (i % 5) * 4;
        c[i] = i * 2;
        d[i] = i * 3;
    }
}

/* GT_EXPR test: > operator */
__attribute__((noinline, pure))
void test_gt(int * restrict dst, const int * restrict src1, const int * restrict src2,
             const int * restrict val1, const int * restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR test: >= operator */
__attribute__((noinline, pure))
void test_ge(int * restrict dst, const int * restrict src1, const int * restrict src2,
             const int * restrict val1, const int * restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR test: < operator */
__attribute__((noinline, pure))
void test_lt(int * restrict dst, const int * restrict src1, const int * restrict src2,
             const int * restrict val1, const int * restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR test: <= operator */
__attribute__((noinline, pure))
void test_le(int * restrict dst, const int * restrict src1, const int * restrict src2,
             const int * restrict val1, const int * restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int * restrict dst, const int * restrict src1, 
                           const int * restrict src2, const int * restrict val1,
                           const int * restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; i++) {
        /* Inner loop with LE_EXPR */
        for (int j = 0; j < 2; j++) {
            int idx = i*2 + j;
            dst[idx] = (src1[idx] > src2[idx]) ? 
                      ((val1[idx] <= val2[idx]) ? val1[idx] : val2[idx]) : 
                      val2[idx];
        }
    }
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int * restrict dst, const int * restrict src1, const int * restrict src2,
                   const int * restrict val1, const int * restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int * restrict dst, const int * restrict src1,
                               const int * restrict src2, const int * restrict val1,
                               const int * restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], const int src2[][8],
                    const int val1[][8], const int val2[][8]) {
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            /* Use GT_EXPR for rows, LE_EXPR for columns */
            dst[i][j] = (src1[i][j] > src2[i][j]) ? 
                       ((i <= j) ? val1[i][j] : val2[i][j]) : 
                       val2[i][j];
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
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N];
    ALIGNED int dst_mixed[N], dst_omp[N], dst_unroll[N];
    ALIGNED int dst_multi[N/8][8];
    ALIGNED int src1_multi[N/8][8], src2_multi[N/8][8];
    ALIGNED int val1_multi[N/8][8], val2_multi[N/8][8];
    
    int total_checksum = 0;
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            int idx = i*8 + j;
            src1_multi[i][j] = src1[idx];
            src2_multi[i][j] = src2[idx];
            val1_multi[i][j] = val1[idx];
            val2_multi[i][j] = val2[idx];
        }
    }
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst_mixed, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_mixed, N);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst_omp, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_omp, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst_unroll, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst_unroll, N);
    
    /* Test multi-dimensional */
    test_multi_dim(dst_multi, src1_multi, src2_multi, val1_multi, val2_multi);
    for (int i = 0; i < N/8; i++) {
        total_checksum += compute_checksum(dst_multi[i], 8);
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
