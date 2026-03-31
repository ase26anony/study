#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        a[i] = i % 17;
        b[i] = (i % 13) * 2;
        c[i] = i * 3;
        d[i] = i * 5;
    }
}

/* GT_EXPR test: > operator */
__attribute__((noinline))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR test: >= operator */
__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR test: < operator */
__attribute__((noinline))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR test: <= operator */
__attribute__((noinline))
void test_le(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparisons in nested context */
__attribute__((noinline))
void test_mixed_nested(int *restrict dst, const int *restrict src1,
                       const int *restrict src2, const int *restrict val1,
                       const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        /* Inner conditional with LE_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = val1[i];
        } else {
            /* Another conditional assignment with different operator */
            dst[i] = (src1[i] <= src2[i]) ? val2[i] : (val1[i] + val2[i]);
        }
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], 
                    const int src2[][8], int rows) {
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < 8; j++) {
            /* GT_EXPR for first dimension, LE_EXPR for second */
            if (i > 0) {
                dst[i][j] = (src1[i][j] <= src2[i][j]) ? src1[i][j] : src2[i][j];
            } else {
                dst[i][j] = (src1[i][j] > src2[i][j]) ? src1[i][j] : src2[i][j];
            }
        }
    }
}

/* Test with OpenMP SIMD pragma */
__attribute__((noinline))
void test_omp_simd(int *restrict dst, const int *restrict src1,
                   const int *restrict src2, const int *restrict val1,
                   const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test with unroll pragma before vectorization */
__attribute__((noinline))
void test_unroll_vectorize(int *restrict dst, const int *restrict src1,
                           const int *restrict src2, const int *restrict val1,
                           const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
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
    ALIGNED int dst5[N], dst6[N], dst7[N];
    ALIGNED int md_src1[64][8], md_src2[64][8], md_dst[64][8];
    
    int total_checksum = 0;
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 8; j++) {
            md_src1[i][j] = (i * 8 + j) % 19;
            md_src2[i][j] = (i * 8 + j) % 13;
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
    
    /* Test mixed/nested comparisons */
    test_mixed_nested(dst5, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst5, N);
    
    /* Test multi-dimensional */
    test_multi_dim(md_dst, md_src1, md_src2, 64);
    for (int i = 0; i < 64; i++) {
        total_checksum += compute_checksum(md_dst[i], 8);
    }
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst6, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst6, N);
    
    /* Test with unroll pragma */
    test_unroll_vectorize(dst7, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst7, N);
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
