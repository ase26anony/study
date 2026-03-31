#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Helper to prevent dead code elimination */
static volatile int sink = 0;

/* Initialize arrays with varying patterns to create mix of true/false conditions */
void init_arrays(int *restrict src1, int *restrict src2, 
                 int *restrict val1, int *restrict val2) {
    for (int i = 0; i < N; i++) {
        src1[i] = (i % 13) * 3;
        src2[i] = (i % 7) * 5;
        val1[i] = i * 2;
        val2[i] = i * 3;
    }
}

/* Test GT_EXPR (>) */
__attribute__((noinline, pure))
int test_gt_expr(int *restrict dst, const int *restrict src1, 
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
int test_ge_expr(int *restrict dst, const int *restrict src1,
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
int test_lt_expr(int *restrict dst, const int *restrict src1,
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
int test_le_expr(int *restrict dst, const int *restrict src1,
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
        
        /* Inner conditional with LE_EXPR - different comparison type */
        if (i % 2 == 0) {
            dst2[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
        } else {
            dst2[i] = (src1[i] > src2[i]) ? val2[i] : val1[i];
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
    const int ROWS = 32;
    const int COLS = 32;
    int sum = 0;
    
    /* Row processing with GT_EXPR */
    for (int i = 0; i < ROWS; i++) {
        #pragma omp simd
        for (int j = 0; j < COLS; j++) {
            int idx = i * COLS + j;
            dst[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
        }
    }
    
    /* Column processing with LE_EXPR */
    for (int j = 0; j < COLS; j++) {
        for (int i = 0; i < ROWS; i++) {
            int idx = i * COLS + j;
            if (i > 0) {  /* Avoid simple pattern */
                dst[idx] += (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
            }
        }
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        sum += dst[i];
    }
    return sum;
}

/* Test with unsigned types to ensure integer comparisons */
__attribute__((noinline, pure))
int test_unsigned_comparisons(unsigned int *restrict dst,
                              const unsigned int *restrict src1,
                              const unsigned int *restrict src2,
                              const unsigned int *restrict val1,
                              const unsigned int *restrict val2) {
    int sum = 0;
    
    /* Mix of comparison operators */
    for (int i = 0; i < N; i++) {
        if (i % 3 == 0) {
            dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        } else if (i % 3 == 1) {
            dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
        } else {
            dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
        }
    }
    
    for (int i = 0; i < N; i++) {
        sum += (int)dst[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N];
    ALIGNED unsigned int usrc1[N], usrc2[N], uval1[N], uval2[N], udst[N];
    
    int total_sum = 0;
    
    /* Initialize arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize unsigned arrays */
    for (int i = 0; i < N; i++) {
        usrc1[i] = (unsigned int)(i * 3);
        usrc2[i] = (unsigned int)(i * 2 + 1);
        uval1[i] = (unsigned int)(i * 5);
        uval2[i] = (unsigned int)(i * 7);
    }
    
    /* Test each comparison operator */
    total_sum += test_gt_expr(dst1, src1, src2, val1, val2);
    total_sum += test_ge_expr(dst2, src1, src2, val1, val2);
    total_sum += test_lt_expr(dst3, src1, src2, val1, val2);
    total_sum += test_le_expr(dst4, src1, src2, val1, val2);
    
    /* Test mixed comparisons */
    total_sum += test_mixed_comparisons(dst1, dst2, src1, src2, val1, val2);
    
    /* Test multi-dimensional */
    total_sum += test_multi_dim(dst5, src1, src2, val1, val2);
    
    /* Test unsigned comparisons */
    total_sum += test_unsigned_comparisons(udst, usrc1, usrc2, uval1, uval2);
    
    /* Use sink to prevent optimization */
    sink = total_sum;
    
    printf("Total checksum: %d\n", total_sum);
    
    return 0;
}
