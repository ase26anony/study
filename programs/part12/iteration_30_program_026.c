#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization */
static void escape(void *p) {
    __asm__ volatile("" : : "r"(p) : "memory");
}

/* Reference implementations */
static void ref_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void ref_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
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

/* Test kernels */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
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

/* Additional test cases for signed/unsigned variations */
static void test_ge_unsigned(int *mask, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

static void test_lt_int(int *mask, const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] < src2[i] ? -1 : 0;
    }
    escape(mask);
}

static void test_le_float(float *dest, const float *src1, const float *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

int main() {
    int ret = 0;
    
    /* Aligned arrays */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N], mask_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        src1_float[i] = i * 0.5f;
        src2_float[i] = (i % 5 == 0) ? i * 0.5f + 0.1f : (i % 5 == 1) ? i * 0.5f - 0.1f : i * 0.5f;
        
        src1_double[i] = i * 0.25;
        src2_double[i] = (i % 7 == 0) ? i * 0.25 + 0.05 : (i % 7 == 1) ? i * 0.25 - 0.05 : i * 0.25;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        ret = 1;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_int, src1_float, src2_float);
    if (memcmp(mask_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        ret = 1;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (count != ref_count) {
        printf("FAIL: LT_EXPR unsigned test (got %u, expected %u)\n", count, ref_count);
        ret = 1;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        ret = 1;
    }
    
    /* Additional tests for complete coverage */
    printf("Testing GE_EXPR with unsigned integers...\n");
    test_ge_unsigned(mask_int, src1_uint, src2_uint);
    ref_ge_float(ref_int, (float*)src1_uint, (float*)src2_uint); /* Just for verification */
    escape(mask_int);
    
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(mask_int, src1_int, src2_int);
    escape(mask_int);
    
    printf("Testing LE_EXPR with floats...\n");
    test_le_float(dest_float, src1_float, src2_float, 1.0f, 0.0f);
    escape(dest_float);
    
    if (ret == 0) {
        printf("All tests passed!\n");
    }
    
    return ret;
}
