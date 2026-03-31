#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns */
void init_arrays(int *restrict src1, int *restrict src2, 
                 int *restrict val1, int *restrict val2) {
    for (int i = 0; i < N; i++) {
        src1[i] = (i % 7) * 3;
        src2[i] = (i % 5) * 4;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) */
__attribute__((noinline, pure))
int test_gt(int *restrict dst, const int *restrict src1, 
            const int *restrict src2, const int *restrict val1,
            const int *restrict val2) {
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
int test_ge(int *restrict dst, const int *restrict src1,
            const int *restrict src2, const int *restrict val1,
            const int *restrict val2) {
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
int test_lt(int *restrict dst, const int *restrict src1,
            const int *restrict src2, const int *restrict val1,
            const int *restrict val2) {
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
int test_le(int *restrict dst, const int *restrict src1,
            const int *restrict src2, const int *restrict val1,
            const int *restrict val2) {
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
int test_mixed_comparisons(int *restrict dst1, int *restrict dst2,
                           const int *restrict src1, const int *restrict src2,
                           const int *restrict val1, const int *restrict val2) {
    int sum = 0;
    
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner loop with LE_EXPR - different comparison type */
        for (int j = 0; j < 8; j++) {
            int idx = i * 8 + j;
            if (idx < N) {
                dst2[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
            }
        }
    }
    
    for (int i = 0; i < N; i++) {
        sum += dst1[i] + dst2[i];
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
    
    /* Process rows with GT_EXPR */
    for (int i = 0; i < ROWS; i++) {
        #pragma omp simd
        for (int j = 0; j < COLS; j++) {
            int idx = i * COLS + j;
            dst[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
        }
    }
    
    /* Process columns with LE_EXPR */
    for (int j = 0; j < COLS; j++) {
        for (int i = 0; i < ROWS; i++) {
            int idx = i * COLS + j;
            dst[idx] += (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
        }
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test with unsigned types to ensure integer comparisons */
__attribute__((noinline, pure))
int test_unsigned(unsigned *restrict dst, const unsigned *restrict src1,
                  const unsigned *restrict src2, const unsigned *restrict val1,
                  const unsigned *restrict val2) {
    int sum = 0;
    for (unsigned i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
    
    for (unsigned i = 0; i < N; i++) {
        sum += (int)dst[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N], dst6[N];
    ALIGNED unsigned usrc1[N], usrc2[N], uval1[N], uval2[N], udst[N];
    
    int total_sum = 0;
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize unsigned arrays */
    for (int i = 0; i < N; i++) {
        usrc1[i] = (unsigned)(i % 11) * 5;
        usrc2[i] = (unsigned)(i % 13) * 3;
        uval1[i] = (unsigned)i * 7;
        uval2[i] = (unsigned)i * 11;
    }
    
    /* Test each comparison operator */
    total_sum += test_gt(dst1, src1, src2, val1, val2);
    total_sum += test_ge(dst2, src1, src2, val1, val2);
    total_sum += test_lt(dst3, src1, src2, val1, val2);
    total_sum += test_le(dst4, src1, src2, val1, val2);
    
    /* Test mixed comparisons */
    total_sum += test_mixed_comparisons(dst5, dst6, src1, src2, val1, val2);
    
    /* Test multi-dimensional */
    total_sum += test_multi_dim(dst1, src1, src2, val1, val2);
    
    /* Test unsigned */
    total_sum += test_unsigned(udst, usrc1, usrc2, uval1, uval2);
    
    /* Use volatile sink to prevent optimization */
    sink = total_sum;
    
    printf("Total checksum: %d\n", total_sum);
    return 0;
}
