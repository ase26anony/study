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

/* Test GT_EXPR (>) - maps to BIT_NOT_EXPR, BIT_AND_EXPR */
__attribute__((noinline, pure))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR */
__attribute__((noinline, pure))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) - maps to BIT_NOT_EXPR, BIT_AND_EXPR with swap */
__attribute__((noinline, pure))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
__attribute__((noinline, pure))
void test_le(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst1, int *restrict dst2,
                           const int *restrict src1, const int *restrict src2,
                           const int *restrict val1, const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        dst1[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
        
        /* Inner loop with LE_EXPR - different comparison type */
        #pragma GCC unroll 4
        for (int j = 0; j < 4; j++) {
            int idx = i * 4 + j;
            if (idx < N) {
                dst2[idx] = (src1[idx] <= src2[idx]) ? val1[idx] : val2[idx];
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

/* Multi-dimensional array with different comparison types per dimension */
__attribute__((noinline))
void test_multi_dim(int *restrict dst, const int *restrict src1,
                    const int *restrict src2, const int *restrict val1,
                    const int *restrict val2) {
    const int ROWS = 32;
    const int COLS = 32;
    
    for (int i = 0; i < ROWS; i++) {
        /* First dimension uses GT_EXPR */
        for (int j = 0; j < COLS; j++) {
            int idx = i * COLS + j;
            dst[idx] = (src1[idx] > src2[idx]) ? val1[idx] : val2[idx];
        }
        
        /* Second pass on same data with LE_EXPR */
        for (int j = 0; j < COLS; j++) {
            int idx = i * COLS + j;
            dst[idx] += (src1[idx] <= src2[idx]) ? val2[idx] : val1[idx];
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

int main(void) {
    /* Use aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N], dst6[N], dst7[N];
    
    /* Initialize source arrays */
    init_arrays(src1, src2, val1, val2);
    
    int total_checksum = 0;
    
    /* Test each comparison operator separately */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, dst6, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst5, N);
    total_checksum += compute_checksum(dst6, N);
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst7, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst7, N);
    
    /* Test multi-dimensional */
    test_multi_dim(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    /* Additional test with unsigned types to ensure different code paths */
    {
        ALIGNED unsigned int usrc1[N], usrc2[N], uval1[N], uval2[N], udst[N];
        for (int i = 0; i < N; i++) {
            usrc1[i] = (unsigned int)(i * 3);
            usrc2[i] = (unsigned int)(i * 2 + 1);
            uval1[i] = (unsigned int)(i * 5);
            uval2[i] = (unsigned int)(i * 7);
        }
        
        for (int i = 0; i < N; i++) {
            udst[i] = (usrc1[i] > usrc2[i]) ? uval1[i] : uval2[i];
        }
        total_checksum += compute_checksum((int*)udst, N);
    }
    
    /* Print result to prevent optimization */
    printf("Total checksum: %d\n", total_checksum);
    
    return 0;
}
