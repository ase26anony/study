#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns to create mixed conditions */
void init_arrays(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; i++) {
        a[i] = i % 7;           /* Pattern 0-6 */
        b[i] = (i % 5) + 2;     /* Pattern 2-6 */
        c[i] = i * 3;           /* Increasing values */
        d[i] = i * 2;           /* Different increasing values */
    }
}

/* Test GT_EXPR (>) */
__attribute__((noinline, pure))
void test_gt(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
void test_ge(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) */
__attribute__((noinline, pure))
void test_lt(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) */
__attribute__((noinline, pure))
void test_le(int* restrict dst, const int* restrict src1, const int* restrict src2,
             const int* restrict val1, const int* restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context - stresses pattern matching */
__attribute__((noinline))
void test_mixed_comparisons(int* restrict dst, const int* restrict a, 
                           const int* restrict b, const int* restrict c) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; i++) {
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < N) {
                /* First conditional with > */
                int temp = (a[idx] > b[idx]) ? c[idx] : a[idx];
                /* Second conditional with <= in same iteration */
                dst[idx] = (temp <= c[idx]) ? b[idx] : temp;
            }
        }
    }
}

/* Test with OpenMP SIMD pragma - different vectorization path */
__attribute__((noinline))
void test_omp_simd(int* restrict dst, const int* restrict src1, 
                   const int* restrict src2, const int* restrict val1,
                   const int* restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma followed by vectorization */
__attribute__((noinline))
void test_unroll_then_vectorize(int* restrict dst, const int* restrict src1,
                               const int* restrict src2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
}

/* Multi-dimensional array with different comparison types per dimension */
__attribute__((noinline))
void test_multi_dim(int dst[][16], const int a[][16], const int b[][16]) {
    int rows = N / 16;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 16; j++) {
            /* Row condition uses GT_EXPR, column condition uses LT_EXPR */
            int row_cond = (i > rows/2) ? a[i][j] : b[i][j];
            dst[i][j] = (j < 8) ? row_cond : (a[i][j] + b[i][j]);
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(const int* arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] ^ i;
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N];
    ALIGNED int dst_mixed[N], dst_omp[N], dst_unroll[N];
    ALIGNED int dst_multi[N/16][16];
    ALIGNED int a_multi[N/16][16], b_multi[N/16][16];
    
    int total_checksum = 0;
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < N/16; i++) {
        for (int j = 0; j < 16; j++) {
            a_multi[i][j] = i * 16 + j;
            b_multi[i][j] = (i * 16 + j) * 2;
        }
    }
    
    /* Test each comparison operator in isolation */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total_checksum += checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    total_checksum += checksum(dst4, N);
    
    /* Test mixed comparison types */
    test_mixed_comparisons(dst_mixed, src1, src2, val1);
    total_checksum += checksum(dst_mixed, N);
    
    /* Test with OpenMP SIMD pragma */
    test_omp_simd(dst_omp, src1, src2, val1, val2);
    total_checksum += checksum(dst_omp, N);
    
    /* Test with unroll pragma */
    test_unroll_then_vectorize(dst_unroll, src1, src2);
    total_checksum += checksum(dst_unroll, N);
    
    /* Test multi-dimensional with different comparisons */
    test_multi_dim(dst_multi, a_multi, b_multi);
    for (int i = 0; i < N/16; i++) {
        total_checksum += checksum(dst_multi[i], 16);
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All conditional vectorization tests completed.\n");
    
    return 0;
}
