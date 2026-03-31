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
__attribute__((noinline))
void test_gt(int *restrict dst, const int *restrict src1, 
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    #pragma GCC unroll 4
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* Test GE_EXPR (>=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR */
__attribute__((noinline))
void test_ge(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LT_EXPR (<) - maps to BIT_NOT_EXPR, BIT_AND_EXPR with swap */
__attribute__((noinline))
void test_lt(int *restrict dst, const int *restrict src1,
             const int *restrict src2, const int *restrict val1,
             const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* Test LE_EXPR (<=) - maps to BIT_NOT_EXPR, BIT_IOR_EXPR with swap */
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
void test_mixed_comparisons(int *restrict dst, const int *restrict src1,
                            const int *restrict src2, const int *restrict val1,
                            const int *restrict val2) {
    /* Outer loop with GT_EXPR */
    for (int i = 0; i < N/2; i++) {
        /* Inner loop with LE_EXPR - stresses pattern matching */
        for (int j = 0; j < 2; j++) {
            int idx = i*2 + j;
            dst[idx] = (src1[idx] > src2[idx]) ? 
                       ((src1[idx] <= val2[idx]) ? val1[idx] : val2[idx]) :
                       val2[idx];
        }
    }
}

/* Multi-dimensional array with different comparison types */
__attribute__((noinline))
void test_multi_dim(int dst[][8], const int src1[][8], 
                    const int src2[][8], int rows) {
    for (int i = 0; i < rows; i++) {
        /* First dimension uses GT_EXPR */
        for (int j = 0; j < 8; j++) {
            dst[i][j] = (src1[i][j] > src2[i][j]) ? 1 : 0;
        }
        /* Second pass with LE_EXPR */
        for (int j = 0; j < 8; j++) {
            dst[i][j] += (src1[i][j] <= src2[i][j]) ? 2 : 0;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i] ^ i;
    }
    return sum;
}

int main() {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N], src2[N], val1[N], val2[N];
    ALIGNED int dst1[N], dst2[N], dst3[N], dst4[N], dst5[N];
    ALIGNED int md_dst[16][8], md_src1[16][8], md_src2[16][8];
    
    int total_checksum = 0;
    
    /* Initialize all arrays */
    init_arrays(src1, src2, val1, val2);
    
    /* Initialize multi-dimensional arrays */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 8; j++) {
            md_src1[i][j] = (i * 8 + j) % 17;
            md_src2[i][j] = (i * 8 + j) % 13;
        }
    }
    
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
    test_mixed_comparisons(dst5, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst5, N);
    
    /* Test multi-dimensional case */
    test_multi_dim(md_dst, md_src1, md_src2, 16);
    for (int i = 0; i < 16; i++) {
        total_checksum += compute_checksum(md_dst[i], 8);
    }
    
    /* Use volatile sink to prevent optimization */
    sink = total_checksum;
    
    printf("Total checksum: %d\n", total_checksum);
    return 0;
}
