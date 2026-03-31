/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
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
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, 
                          double true_val, double false_val) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? true_val : false_val;
    }
}

/* Test kernels designed to trigger vector comparison transformations */

/* GT_EXPR with integers - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR with floats - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* LT_EXPR with unsigned integers - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* LE_EXPR with doubles - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_le_double(double *dest, const double *src1, const double *src2,
                           double true_val, double false_val) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? true_val : false_val;
    }
    escape(dest);
}

/* Additional test cases to ensure coverage */

/* GT_EXPR with mixed pattern for mask generation */
static void test_gt_mask(char *mask, const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] > src2[i];
    }
    escape(mask);
}

/* GE_EXPR with unsigned for reduction */
static unsigned test_ge_reduce(const unsigned *src1, const unsigned *src2) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (src1[i] >= src2[i]) ? src1[i] : 0;
    }
    escape(&sum);
    return sum;
}

/* LT_EXPR with floats in conditional assignment */
static void test_lt_float_cond(float *dest, const float *src1, const float *src2,
                               float threshold) {
    for (int i = 0; i < N; i++) {
        if (src1[i] < src2[i]) {
            dest[i] = src1[i] * threshold;
        } else {
            dest[i] = src2[i];
        }
    }
    escape(dest);
}

/* LE_EXPR with integers in complex expression */
static void test_le_int_complex(int *dest, const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = (src1[i] <= src2[i]) ? (src1[i] + src2[i]) : (src1[i] - src2[i]);
    }
    escape(dest);
}

int main() {
    int ret = 0;
    
    /* Aligned arrays for vector loads */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    ALIGNED char mask[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating pattern */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 5 : i * 2 - 3;
        
        /* Float arrays: create some NaN/inf values to test special cases */
        src1_float[i] = (i % 7 == 0) ? (float)i * 1.5f : (float)i;
        src2_float[i] = (i % 5 == 0) ? (float)i * 0.8f : (float)i + 0.5f;
        
        /* Double arrays: similar pattern */
        src1_double[i] = (i % 6 == 0) ? (double)i * 2.0 : (double)i;
        src2_double[i] = (i % 4 == 0) ? (double)i * 1.5 : (double)i + 1.0;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, -100);
    ref_gt_int(ref_int, src1_int, src2_int, 100, -100);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        ret = 1;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        ret = 1;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count_vec = test_lt_unsigned(src1_uint, src2_uint);
    unsigned count_ref = ref_lt_unsigned(src1_uint, src2_uint);
    if (count_vec != count_ref) {
        printf("FAIL: LT_EXPR unsigned test: vec=%u, ref=%u\n", count_vec, count_ref);
        ret = 1;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 99.9, -99.9);
    ref_le_double(ref_double, src1_double, src2_double, 99.9, -99.9);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        ret = 1;
    }
    
    printf("Testing additional patterns...\n");
    
    /* Test GT mask generation */
    test_gt_mask(mask, src1_int, src2_int);
    escape(mask);
    
    /* Test GE reduction */
    unsigned sum_vec = test_ge_reduce(src1_uint, src2_uint);
    escape(&sum_vec);
    
    /* Test LT float conditional */
    test_lt_float_cond(dest_float, src1_float, src2_float, 2.0f);
    escape(dest_float);
    
    /* Test LE complex integer expression */
    test_le_int_complex(dest_int, src1_int, src2_int);
    escape(dest_int);
    
    if (ret == 0) {
        printf("All tests passed!\n");
    }
    
    return ret;
}
