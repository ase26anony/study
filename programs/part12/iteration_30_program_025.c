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

static uint32_t ref_lt_unsigned(const uint32_t *src1, const uint32_t *src2) {
    uint32_t count = 0;
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

static uint32_t test_lt_unsigned(const uint32_t *src1, const uint32_t *src2) {
    uint32_t count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
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

/* Additional tests for signed integer comparisons */
static void test_lt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Test with mixed patterns to ensure both true and false comparisons */
static void init_test_data(ALIGNED int *src1_int, ALIGNED int *src2_int,
                           ALIGNED uint32_t *src1_uint, ALIGNED uint32_t *src2_uint,
                           ALIGNED float *src1_float, ALIGNED float *src2_float,
                           ALIGNED double *src1_double, ALIGNED double *src2_double) {
    for (int i = 0; i < N; i++) {
        /* Create alternating patterns for mixed comparison results */
        src1_int[i] = (i % 4 == 0) ? i + 10 : i - 5;
        src2_int[i] = i;
        
        src1_uint[i] = (i % 3 == 0) ? i + 7 : i - 3;
        src2_uint[i] = i;
        
        src1_float[i] = (i % 5 == 0) ? i * 1.5f : i * 0.8f;
        src2_float[i] = i * 1.0f;
        
        src1_double[i] = (i % 6 == 0) ? i * 2.0 : i * 0.5;
        src2_double[i] = i * 1.0;
    }
}

int main() {
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED uint32_t src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    ALIGNED int dest_int[N], ref_int[N];
    ALIGNED float dest_float[N], ref_float[N];
    ALIGNED double dest_double[N], ref_double[N];
    
    int errors = 0;
    
    /* Initialize test data with mixed patterns */
    init_test_data(src1_int, src2_int, src1_uint, src2_uint,
                   src1_float, src2_float, src1_double, src2_double);
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, -100);
    ref_gt_int(ref_int, src1_int, src2_int, 100, -100);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction pattern) */
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    uint32_t vec_count = test_lt_unsigned(src1_uint, src2_uint);
    uint32_t ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed: %u vs %u\n", 
               vec_count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, -2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, -2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Additional tests for signed integer LT and LE */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 777, 999);
    ref_gt_int(ref_int, src2_int, src1_int, 777, 999); /* a < b == b > a */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed integer test failed\n");
        errors++;
    }
    
    printf("Testing LE_EXPR with signed integers...\n");
    test_le_int(dest_int, src1_int, src2_int, 555, 444);
    ref_ge_int(ref_int, src2_int, src1_int, 555, 444); /* a <= b == b >= a */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LE_EXPR signed integer test failed\n");
        errors++;
    }
    
    /* Summary */
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}

/* Helper reference function for LE test */
static void ref_ge_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
}
