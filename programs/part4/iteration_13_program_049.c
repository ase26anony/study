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
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR */
__attribute__((noinline, pure))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    #pragma omp simd
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
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
__attribute__((noinline))
void test_mixed_comparisons(int *restrict dst, const int *restrict src1,
                            const int *restrict src2, const int *restrict val1,
                            const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N; i++) {
        /* Inner conditional with LE_EXPR - may cause multiple passes */
        if (src1[i] > src2[i]) {
            dst[i] = (val1[i] <= val2[i]) ? val1[i] * 2 : val2[i] * 2;
        } else {
            dst[i] = (val1[i] <= val2[i]) ? val1[i] / 2 : val2[i] / 2;
        }
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][16], const int src1[][16], 
                    const int src2[][16], int rows) {
    for (int i = 0; i < rows; i++) {
        /* First dimension uses GT_EXPR */
        for (int j = 0; j < 16; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? i + j : i - j;
        }
        
        /* Second pass on same data with LE_EXPR */
        for (int j = 0; j < 16; j++) {
            dst[i][j] += (src1[i][j] <= src2[i][j]) ? i * j : i / (j + 1);
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(const int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N];
    ALIGNED int mdst[64][16], msrc1[64][16], msrc2[64][16];
    
    int total = 0;
    
    /* Initialize source arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Test each comparison operator in isolation */
    test_gt(dst1, src1, src2, val1, val2);
    total += checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total += checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total += checksum(dst3, N);
    
    test_le(dst4, src1, src2, val1, val2);
    total += checksum(dst4, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst5, src1, src2, val1, val2);
    total += checksum(dst5, N);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            msrc1[i][j] = (i * 17 + j * 13) % 100;
            msrc2[i][j] = (i * 11 + j * 19) % 100;
        }
    }
    
    /* Test multi-dimensional case */
    test_multi_dim(mdst, msrc1, msrc2, 64);
    
    /* Add multi-dimensional checksum */
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            total += mdst[i][j];
        }
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total;
    
    printf("Total checksum: %d\n", total);
    return 0;
}
