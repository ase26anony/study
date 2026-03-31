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

/* Reference implementations for verification */
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
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered transformations */

/* GT_EXPR transformation for integers */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR transformation for floats */
static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* LT_EXPR transformation for unsigned integers (with potential swap) */
static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* LE_EXPR transformation for doubles (with potential swap) */
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

static void test_ge_unsigned(int *mask, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Mixed pattern to ensure various comparison results */
static void initialize_data(int *int_src1, int *int_src2,
                           unsigned *uint_src1, unsigned *uint_src2,
                           float *float_src1, float *float_src2,
                           double *double_src1, double *double_src2) {
    for (int i = 0; i < N; i++) {
        /* Create alternating patterns for mixed comparison results */
        int_src1[i] = i;
        int_src2[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        uint_src1[i] = i * 2;
        uint_src2[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        float_src1[i] = i * 1.5f;
        float_src2[i] = (i % 5 == 0) ? i * 1.5f + 0.5f : 
                       (i % 5 == 1) ? i * 1.5f - 0.5f : i * 1.5f;
        
        double_src1[i] = i * 2.5;
        double_src2[i] = (i % 6 == 0) ? i * 2.5 + 1.0 :
                        (i % 6 == 1) ? i * 2.5 - 1.0 : i * 2.5;
    }
}

int main() {
    /* Aligned arrays for vector loads */
    ALIGNED int int_src1[N], int_src2[N];
    ALIGNED unsigned uint_src1[N], uint_src2[N];
    ALIGNED float float_src1[N], float_src2[N];
    ALIGNED double double_src1[N], double_src2[N];
    
    /* Destination arrays */
    ALIGNED int int_dest[N], int_dest_ref[N];
    ALIGNED int int_mask[N], int_mask_ref[N];
    ALIGNED double double_dest[N], double_dest_ref[N];
    
    /* Initialize test data */
    initialize_data(int_src1, int_src2, uint_src1, uint_src2,
                   float_src1, float_src2, double_src1, double_src2);
    
    int errors = 0;
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(int_dest, int_src1, int_src2, 100, 200);
    ref_gt_int(int_dest_ref, int_src1, int_src2, 100, 200);
    if (memcmp(int_dest, int_dest_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(int_mask, float_src1, float_src2);
    ref_ge_float(int_mask_ref, float_src1, float_src2);
    if (memcmp(int_mask, int_mask_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers */
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count = test_lt_unsigned(uint_src1, uint_src2);
    unsigned ref_count = ref_lt_unsigned(uint_src1, uint_src2);
    if (count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: %u != %u\n", count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(double_dest, double_src1, double_src2, 3.14159, 2.71828);
    ref_le_double(double_dest_ref, double_src1, double_src2, 3.14159, 2.71828);
    if (memcmp(double_dest, double_dest_ref, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Test 5: LT_EXPR with signed integers (covers swap case) */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(int_dest, int_src1, int_src2, 300, 400);
    /* Reference implementation for LT */
    for (int i = 0; i < N; i++) {
        int_dest_ref[i] = int_src1[i] < int_src2[i] ? 300 : 400;
    }
    if (memcmp(int_dest, int_dest_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed integer test failed\n");
        errors++;
    }
    
    /* Test 6: GE_EXPR with unsigned integers */
    printf("Testing GE_EXPR with unsigned integers...\n");
    test_ge_unsigned(int_mask, uint_src1, uint_src2);
    /* Reference implementation for GE unsigned */
    for (int i = 0; i < N; i++) {
        int_mask_ref[i] = uint_src1[i] >= uint_src2[i] ? -1 : 0;
    }
    if (memcmp(int_mask, int_mask_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR unsigned test failed\n");
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
