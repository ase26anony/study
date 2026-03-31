#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent dead code elimination */
static void escape(void *p) {
    __asm__ volatile("" : : "r"(p) : "memory");
}

/* Reference scalar implementations for verification */
static void ref_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void ref_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

static unsigned ref_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered patterns */

/* GT_EXPR with integers - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? 0xFFFFFFFF : 0x00000000;
    }
    escape(dest);
}

/* GE_EXPR with floats - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_ge_float(float ALIGNED *dest, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* LT_EXPR with unsigned integers - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static unsigned test_lt_unsigned(const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* LE_EXPR with doubles - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? 3.141592653589793 : 2.718281828459045;
    }
    escape(dest);
}

/* Additional test with mixed comparisons to ensure all paths are exercised */
static void test_mixed_comparisons(int ALIGNED *dest_int, float ALIGNED *dest_float,
                                   const int ALIGNED *src1_int, const int ALIGNED *src2_int,
                                   const float ALIGNED *src1_float, const float ALIGNED *src2_float) {
    /* GT with integers */
    for (int i = 0; i < N; i++) {
        dest_int[i] = src1_int[i] > src2_int[i] ? 100 : -100;
    }
    
    /* LE with floats */
    for (int i = 0; i < N; i++) {
        dest_float[i] = src1_float[i] <= src2_float[i] ? 10.0f : -10.0f;
    }
    
    escape(dest_int);
    escape(dest_float);
}

int main() {
    int ret = 0;
    
    /* Allocate aligned arrays */
    int *src1_int = aligned_alloc(32, N * sizeof(int));
    int *src2_int = aligned_alloc(32, N * sizeof(int));
    int *dest_int = aligned_alloc(32, N * sizeof(int));
    int *ref_int = aligned_alloc(32, N * sizeof(int));
    
    unsigned *src1_uint = aligned_alloc(32, N * sizeof(unsigned));
    unsigned *src2_uint = aligned_alloc(32, N * sizeof(unsigned));
    
    float *src1_float = aligned_alloc(32, N * sizeof(float));
    float *src2_float = aligned_alloc(32, N * sizeof(float));
    float *dest_float = aligned_alloc(32, N * sizeof(float));
    float *ref_float = aligned_alloc(32, N * sizeof(float));
    
    double *src1_double = aligned_alloc(32, N * sizeof(double));
    double *src2_double = aligned_alloc(32, N * sizeof(double));
    double *dest_double = aligned_alloc(32, N * sizeof(double));
    double *ref_double = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* For integers: alternating pattern */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* For unsigned: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 5 : i * 2 - 3;
        
        /* For floats: sinusoidal pattern */
        src1_float[i] = 1.5f * i;
        src2_float[i] = 1.5f * i + ((i % 5) - 2) * 0.5f;
        
        /* For doubles: exponential pattern */
        src1_double[i] = 0.1 * i;
        src2_double[i] = 0.1 * i + ((i % 7) - 3) * 0.05;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int);
    ref_gt_int(ref_int, src1_int, src2_int, 0xFFFFFFFF, 0x00000000);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test failed\n");
        ret = 1;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GE_EXPR float test failed\n");
        ret = 1;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("FAIL: LT_EXPR unsigned test failed: vec=%u, ref=%u\n", vec_count, ref_count);
        ret = 1;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double);
    ref_le_double(ref_double, src1_double, src2_double, 3.141592653589793, 2.718281828459045);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test failed\n");
        ret = 1;
    }
    
    /* Test mixed comparisons */
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(dest_int, dest_float, src1_int, src2_int, src1_float, src2_float);
    
    /* Cleanup */
    free(src1_int);
    free(src2_int);
    free(dest_int);
    free(ref_int);
    free(src1_uint);
    free(src2_uint);
    free(src1_float);
    free(src2_float);
    free(dest_float);
    free(ref_float);
    free(src1_double);
    free(src2_double);
    free(dest_double);
    free(ref_double);
    
    if (ret == 0) {
        printf("All tests passed!\n");
    }
    
    return ret;
}
