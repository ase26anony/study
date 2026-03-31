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
        a[i] = (i % 7) * 3;
        b[i] = (i % 5) * 5;
        c[i] = (i % 11) * 7;
        d[i] = (i % 13) * 11;
    }
}

/* GT_EXPR test: > operator */
__attribute__((noinline))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR test: >= operator */
__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    #pragma omp simd
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
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed(int *restrict dst, const int *restrict src1,
                const int *restrict src2, const int *restrict val1,
                const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        /* Inner conditional with LE_EXPR */
        if (src1[i] > src2[i]) {
            dst[i] = val1[i];
        } else {
            /* Nested conditional assignment with different comparison */
            dst[i] = (src1[i] <= src2[i]) ? val2[i] : (val1[i] + val2[i]) / 2;
        }
    }
}

/* Multi-dimensional test with different comparison operators */
__attribute__((noinline))
void test_2d(int dst[][8], const int src1[][8], const int src2[][8]) {
    const int rows = N / 8;
    
    /* First dimension uses GT_EXPR */
    for (int i = 0; i < rows; i++) {
        /* Second dimension uses LE_EXPR */
        for (int j = 0; j < 8; j++) {
            dst[i][j] = (i > 0) ? 
                       ((src1[i][j] <= src2[i][j]) ? src1[i][j] : src2[i][j]) :
                       ((src1[i][j] > src2[i][j]) ? src2[i][j] : src1[i][j]);
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N];
    ALIGNED int dst_2d[N/8][8];
    ALIGNED int src1_2d[N/8][8], src2_2d[N/8][8];
    
    int total_checksum = 0;
    
    /* Initialize data */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize 2D arrays */
    for (int i = 0; i < N/8; i++) {
        for (int j = 0; j < 8; j++) {
            src1_2d[i][j] = (i * 8 + j) % 17;
            src2_2d[i][j] = (i * 8 + j) % 13;
        }
    }
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total_checksum += checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    total_checksum += checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed(dst5, src1, src2, val1, val2);
    total_checksum += checksum(dst5, N);
    
    /* Test 2D case with different operators */
    test_2d(dst_2d, src1_2d, src2_2d);
    for (int i = 0; i < N/8; i++) {
        total_checksum += checksum(dst_2d[i], 8);
    }
    
    /* Additional test: chain of conditions with different operators */
    ALIGNED int dst_chain[N];
    for (int i = 0; i < N; i++) {
        /* This creates a pattern that might trigger multiple passes */
        dst_chain[i] = (src1[i] > src2[i]) ? val1[i] :
                      (src1[i] >= src2[i]) ? val2[i] :
                      (src1[i] < src2[i]) ? val1[i] + val2[i] :
                      (src1[i] <= src2[i]) ? val1[i] - val2[i] : 0;
    }
    total_checksum += checksum(dst_chain, N);
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
