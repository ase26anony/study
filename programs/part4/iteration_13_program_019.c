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
        c[i] = (i % 11) * 2;
        d[i] = (i % 13) * 4;
    }
}

/* Test GT_EXPR (>) */
__attribute__((noinline, pure))
int test_gt(int *restrict dst, const int *restrict src1, const int *restrict src2,
            const int *restrict val1, const int *restrict val2) {
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

/* Test GE_EXPR (>=) */
__attribute__((noinline, pure))
int test_ge(int *restrict dst, const int *restrict src1, const int *restrict src2,
            const int *restrict val1, const int *restrict val2) {
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

/* Test LT_EXPR (<) */
__attribute__((noinline, pure))
int test_lt(int *restrict dst, const int *restrict src1, const int *restrict src2,
            const int *restrict val1, const int *restrict val2) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
    
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test LE_EXPR (<=) */
__attribute__((noinline, pure))
int test_le(int *restrict dst, const int *restrict src1, const int *restrict src2,
            const int *restrict val1, const int *restrict val2) {
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

/* Mixed comparison types in nested context */
__attribute__((noinline))
int test_mixed_comparisons(int *restrict dst, const int *restrict src1, 
                          const int *restrict src2, const int *restrict val1,
                          const int *restrict val2) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; i++) {
        /* Inner loop with LE_EXPR - nested to stress pattern matching */
        for (int j = 0; j < 2; j++) {
            int idx = i*2 + j;
            dst[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
        }
    }
    
    /* Second pass with different comparison */
    for (int i = 0; i < N; i++) {
        /* Conditional with LT_EXPR inside loop that also uses other comparisons */
        int temp = (src1[i] < src2[i]) ? val1[i] : val2[i];
        dst[i] += (dst[i] <= temp) ? temp : dst[i];
    }
    
    for (int i = 0; i < N; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
int test_multi_dim(int *restrict dst, const int *restrict src1,
                  const int *restrict src2, const int *restrict val1,
                  const int *restrict val2) {
    int sum = 0;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Process 2D array - row-major order */
    for (int i = 0; i < ROWS; i++) {
        /* Row processing with GE_EXPR */
        #pragma omp simd
        for (int j = 0; j < COLS; j++) {
            int idx = i * COLS + j;
            dst[idx] = (src1[idx] >= src2[idx]) ? val1[idx] : val2[idx];
        }
        
        /* Column processing with different comparison (LE_EXPR) */
        for (int j = 0; j < COLS; j++) {
            int idx = i * COLS + j;
            /* Nested conditional with LE_EXPR */
            if (dst[idx] <= val1[idx]) {
                dst[idx] = (src1[idx] > src2[idx]) ? dst[idx] : val2[idx];
            }
        }
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += dst[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N], dst6[N];
    
    /* Initialize with varying patterns */
    init_arrays(src1, src2, val1, val2);
    
    /* Clear destination arrays */
    memset(dst1, 0, sizeof(dst1));
    memset(dst2, 0, sizeof(dst2));
    memset(dst3, 0, sizeof(dst3));
    memset(dst4, 0, sizeof(dst4));
    memset(dst5, 0, sizeof(dst5));
    memset(dst6, 0, sizeof(dst6));
    
    int total_sum = 0;
    
    /* Test each comparison operator in vectorizable loops */
    total_sum += test_gt(dst1, src1, src2, val1, val2);
    total_sum += test_ge(dst2, src1, src2, val1, val2);
    total_sum += test_lt(dst3, src1, src2, val1, val2);
    total_sum += test_le(dst4, src1, src2, val1, val2);
    
    /* Test mixed/nested comparisons */
    total_sum += test_mixed_comparisons(dst5, src1, src2, val1, val2);
    
    /* Test multi-dimensional case */
    total_sum += test_multi_dim(dst6, src1, src2, val1, val2);
    
    /* Use volatile sink to prevent optimization */
    sink = total_sum;
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
