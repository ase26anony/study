#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent compiler from optimizing away computations */
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
        count += (src1[i] < src2[i]);
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Vectorizable test kernels */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed/unsigned variations */
static void test_lt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_unsigned(unsigned *dest, const unsigned *src1, const unsigned *src2, unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_gt_float(float *dest, const float *src1, const float *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

int main() {
    int result = 0;
    
    /* Aligned arrays with mixed patterns to ensure both true and false comparisons */
    ALIGNED int src1_int[N];
    ALIGNED int src2_int[N];
    ALIGNED int dest_int[N];
    ALIGNED int ref_int[N];
    
    ALIGNED unsigned src1_uint[N];
    ALIGNED unsigned src2_uint[N];
    ALIGNED unsigned dest_uint[N];
    ALIGNED unsigned ref_uint[N];
    
    ALIGNED float src1_float[N];
    ALIGNED float src2_float[N];
    ALIGNED float dest_float[N];
    ALIGNED float ref_float[N];
    
    ALIGNED double src1_double[N];
    ALIGNED double src2_double[N];
    ALIGNED double dest_double[N];
    ALIGNED double ref_double[N];
    
    /* Initialize with patterns that create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer patterns: alternating greater/lesser */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 5 : 
                     (i % 3 == 1) ? i - 2 : i;
        
        /* Unsigned patterns */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float patterns with NaN avoidance */
        src1_float[i] = (i % 5 == 0) ? i * 1.5f : i * 0.75f;
        src2_float[i] = (i % 5 == 0) ? i * 0.75f : i * 1.5f;
        
        /* Double patterns */
        src1_double[i] = (i % 7 == 0) ? i * 2.0 : i * 0.5;
        src2_double[i] = (i % 7 == 0) ? i * 0.5 : i * 2.0;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        result = 1;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        result = 1;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("FAIL: LT_EXPR unsigned reduction test (%u vs %u)\n", vec_count, ref_count);
        result = 1;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        result = 1;
    }
    
    /* Additional tests for complete coverage */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 777, 333);
    ref_gt_int(ref_int, src2_int, src1_int, 777, 333); /* Equivalent to src1 < src2 */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LT_EXPR signed integer test\n");
        result = 1;
    }
    
    printf("Testing LE_EXPR with unsigned integers...\n");
    test_le_unsigned(dest_uint, src1_uint, src2_uint, 0xFFFFFFFF, 0);
    for (int i = 0; i < N; i++) {
        ref_uint[i] = src1_uint[i] <= src2_uint[i] ? 0xFFFFFFFF : 0;
    }
    if (memcmp(dest_uint, ref_uint, N * sizeof(unsigned)) != 0) {
        printf("FAIL: LE_EXPR unsigned test\n");
        result = 1;
    }
    
    printf("Testing GE_EXPR with signed integers...\n");
    test_ge_int(dest_int, src1_int, src2_int, 123, 456);
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] >= src2_int[i] ? 123 : 456;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GE_EXPR signed integer test\n");
        result = 1;
    }
    
    printf("Testing GT_EXPR with floats...\n");
    test_gt_float(dest_float, src1_float, src2_float, 10.0f, -10.0f);
    for (int i = 0; i < N; i++) {
        ref_float[i] = src1_float[i] > src2_float[i] ? 10.0f : -10.0f;
    }
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GT_EXPR float test\n");
        result = 1;
    }
    
    if (result == 0) {
        printf("All tests passed!\n");
    }
    
    return result;
}
