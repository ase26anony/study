/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent compiler from optimizing away computations */
static void escape(void *p) {
    __asm__ volatile ("" : : "r"(p) : "memory");
}

/* Reference scalar implementations for verification */
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

/* Test kernels - should be vectorized */
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

/* Additional tests for signed/unsigned variations */
static void test_gt_unsigned(unsigned *dest, const unsigned *src1, const unsigned *src2, 
                             unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_lt_int(int *mask, const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] < src2[i] ? -1 : 0;
    }
    escape(mask);
}

static void test_ge_double(double *dest, const double *src1, const double *src2, 
                           double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_float(float *dest, const float *src1, const float *src2, 
                          float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vector loads */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N], mask_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N], dest_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: create some NaN/inf patterns */
        src1_float[i] = (float)(i - N/2) * 1.5f;
        src2_float[i] = (float)(i - N/2) * 1.0f;
        
        /* Double arrays */
        src1_double[i] = (double)(i - N/2) * 2.5;
        src2_double[i] = (double)(i - N/2) * 2.0;
    }
    
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
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_int, src1_float, src2_float);
    if (memcmp(mask_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction pattern) */
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed: %u vs %u\n", 
               vec_count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 99.9, -99.9);
    ref_le_double((double*)ref_int, src1_double, src2_double, 99.9, -99.9);
    if (memcmp(dest_double, (double*)ref_int, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Test 5: GT_EXPR with unsigned integers */
    printf("Testing GT_EXPR with unsigned integers...\n");
    test_gt_unsigned(dest_uint, src1_uint, src2_uint, 0xFFFFFFFF, 0);
    /* Reference implementation */
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_uint[i] > src2_uint[i] ? 0xFFFFFFFF : 0;
    }
    if (memcmp(dest_uint, ref_int, N * sizeof(unsigned)) != 0) {
        printf("  ERROR: GT_EXPR unsigned test failed\n");
        errors++;
    }
    
    /* Test 6: LT_EXPR with signed integers */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(mask_int, src1_int, src2_int);
    /* Reference implementation */
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] < src2_int[i] ? -1 : 0;
    }
    if (memcmp(mask_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed test failed\n");
        errors++;
    }
    
    /* Test 7: GE_EXPR with doubles */
    printf("Testing GE_EXPR with doubles...\n");
    test_ge_double(dest_double, src1_double, src2_double, 77.7, -77.7);
    /* Reference implementation */
    for (int i = 0; i < N; i++) {
        double ref_val = src1_double[i] >= src2_double[i] ? 77.7 : -77.7;
        ((double*)ref_int)[i] = ref_val;
    }
    if (memcmp(dest_double, (double*)ref_int, N * sizeof(double)) != 0) {
        printf("  ERROR: GE_EXPR double test failed\n");
        errors++;
    }
    
    /* Test 8: LE_EXPR with floats */
    printf("Testing LE_EXPR with floats...\n");
    test_le_float(dest_float, src1_float, src2_float, 55.5f, -55.5f);
    /* Reference implementation */
    for (int i = 0; i < N; i++) {
        float ref_val = src1_float[i] <= src2_float[i] ? 55.5f : -55.5f;
        ((float*)ref_int)[i] = ref_val;
    }
    if (memcmp(dest_float, (float*)ref_int, N * sizeof(float)) != 0) {
        printf("  ERROR: LE_EXPR float test failed\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
