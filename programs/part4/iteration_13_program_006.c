#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 128

/* Helper functions with no side effects for better vectorization analysis */
__attribute__((const)) static int get_val1(int i) { return i * 3; }
__attribute__((const)) static int get_val2(int i) { return i * 7; }

/* Test functions for each comparison operator */

/* GT_EXPR (>) */
void test_gt(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? val1[i] : val2[i];
    }
}

/* GE_EXPR (>=) */
void test_ge(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* LT_EXPR (<) */
void test_lt(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] < src2[i]) ? val1[i] : val2[i];
    }
}

/* LE_EXPR (<=) */
void test_le(int *restrict dst, const int *restrict src1, const int *restrict src2,
             const int *restrict val1, const int *restrict val2) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] <= src2[i]) ? val1[i] : val2[i];
    }
}

/* Mixed comparison types in nested context */
void test_mixed_comparisons(int *restrict dst, const int *restrict src1, 
                           const int *restrict src2, const int *restrict src3) {
    for (int i = 0; i < N; i++) {
        /* Outer condition uses GT_EXPR */
        if (src1[i] > src2[i]) {
            /* Inner condition uses LE_EXPR */
            dst[i] = (src2[i] <= src3[i]) ? src1[i] : src3[i];
        } else {
            /* Another condition using LT_EXPR */
            dst[i] = (src1[i] < src3[i]) ? src2[i] : src3[i];
        }
    }
}

/* Multi-dimensional array with different comparison types per dimension */
void test_multi_dim(int dst[M][M], const int src1[M][M], const int src2[M][M]) {
    /* First dimension uses GT_EXPR */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 0; j < M; j++) {
            /* Second dimension uses LE_EXPR */
            dst[i][j] = (src1[i][j] > src2[i][j]) ? 
                       ((i <= j) ? src1[i][j] : src2[i][j]) : 
                       ((i > j) ? src1[i][j] * 2 : src2[i][j] * 2);
        }
    }
}

/* Test with OpenMP SIMD pragma */
void test_omp_simd(int *restrict dst, const int *restrict src1, 
                   const int *restrict src2, const int *restrict val1, 
                   const int *restrict val2) {
    #pragma omp simd
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] >= src2[i]) ? val1[i] : val2[i];
    }
}

/* Complex pattern with multiple conditional assignments */
void test_complex_pattern(int *restrict dst, const int *restrict src1,
                         const int *restrict src2, const int *restrict src3) {
    for (int i = 0; i < N; i++) {
        int temp;
        /* First conditional with GT_EXPR */
        temp = (src1[i] > src2[i]) ? src1[i] : src2[i];
        /* Second conditional with LT_EXPR */
        dst[i] = (temp < src3[i]) ? temp : src3[i];
        /* Third conditional with GE_EXPR */
        if (dst[i] >= src1[i]) {
            dst[i] = (src2[i] <= src3[i]) ? dst[i] * 2 : dst[i] / 2;
        }
    }
}

/* Compute checksum to prevent dead code elimination */
unsigned long long compute_checksum(const int *arr, int size) {
    unsigned long long sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (unsigned long long)arr[i];
    }
    return sum;
}

int main() {
    /* Allocate and initialize arrays with varying patterns */
    int *src1 = malloc(N * sizeof(int));
    int *src2 = malloc(N * sizeof(int));
    int *src3 = malloc(N * sizeof(int));
    int *val1 = malloc(N * sizeof(int));
    int *val2 = malloc(N * sizeof(int));
    int *dst1 = malloc(N * sizeof(int));
    int *dst2 = malloc(N * sizeof(int));
    int *dst3 = malloc(N * sizeof(int));
    
    /* Multi-dimensional arrays */
    int (*md_src1)[M] = malloc(M * M * sizeof(int));
    int (*md_src2)[M] = malloc(M * M * sizeof(int));
    int (*md_dst)[M] = malloc(M * M * sizeof(int));
    
    /* Initialize with varying data to create mix of true/false conditions */
    for (int i = 0; i < N; i++) {
        src1[i] = i;
        src2[i] = (i % 3 == 0) ? i + 1 : i - 1;
        src3[i] = (i % 5 == 0) ? i * 2 : i / 2;
        val1[i] = get_val1(i);
        val2[i] = get_val2(i);
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < M; j++) {
            md_src1[i][j] = i * M + j;
            md_src2[i][j] = (i + j) % 7;
        }
    }
    
    unsigned long long total_checksum = 0;
    
    /* Test each comparison operator */
    test_gt(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    test_ge(dst2, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst2, N);
    
    test_lt(dst3, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst3, N);
    
    test_le(dst1, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst1, N);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(dst2, src1, src2, src3);
    total_checksum += compute_checksum(dst2, N);
    
    /* Test multi-dimensional */
    test_multi_dim(md_dst, md_src1, md_src2);
    for (int i = 0; i < M; i++) {
        total_checksum += compute_checksum(md_dst[i], M);
    }
    
    /* Test with OpenMP SIMD */
    test_omp_simd(dst3, src1, src2, val1, val2);
    total_checksum += compute_checksum(dst3, N);
    
    /* Test complex pattern */
    test_complex_pattern(dst1, src1, src2, src3);
    total_checksum += compute_checksum(dst1, N);
    
    /* Additional test with unsigned types (may affect comparison semantics) */
    unsigned int *usrc1 = (unsigned int*)src1;
    unsigned int *usrc2 = (unsigned int*)src2;
    unsigned int *udst = (unsigned int*)dst2;
    
    for (int i = 0; i < N; i++) {
        udst[i] = (usrc1[i] > usrc2[i]) ? usrc1[i] : usrc2[i];
    }
    total_checksum += compute_checksum((int*)udst, N);
    
    printf("Total checksum: %llu\n", total_checksum);
    
    /* Cleanup */
    free(src1); free(src2); free(src3);
    free(val1); free(val2);
    free(dst1); free(dst2); free(dst3);
    free(md_src1); free(md_src2); free(md_dst);
    
    return 0;
}
