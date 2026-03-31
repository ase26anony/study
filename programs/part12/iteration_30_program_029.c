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

static void ref_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
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

/* Vectorizable test functions targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats */
static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
    escape(mask);
}

/* Case 3: LT_EXPR with unsigned integers (reduction pattern) */
static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles */
static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed/unsigned variations */

/* GT_EXPR with unsigned integers */
static void test_gt_unsigned(int *dest, const unsigned *src1, const unsigned *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR with signed integers (reduction) */
static int test_ge_int_reduction(const int *src1, const int *src2) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (src1[i] >= src2[i]);
    }
    escape(&sum);
    return sum;
}

/* LT_EXPR with floats (mask generation) */
static void test_lt_float_mask(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] < src2[i];
    }
    escape(mask);
}

/* LE_EXPR with unsigned integers */
static void test_le_unsigned(int *dest, const unsigned *src1, const unsigned *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vector loads */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N], mask_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned patterns */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 5 : i * 2 - 3;
        
        /* Float patterns */
        src1_float[i] = i * 0.5f;
        src2_float[i] = (i % 5 == 0) ? i * 0.5f + 0.3f : i * 0.5f - 0.2f;
        
        /* Double patterns */
        src1_double[i] = i * 0.25;
        src2_double[i] = (i % 7 == 0) ? i * 0.25 + 0.1 : i * 0.25 - 0.05;
    }
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
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
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
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
    test_le_double(dest_double, src1_double, src2_double, 1.0, -1.0);
    ref_le_double(ref_double, src1_double, src2_double, 1.0, -1.0);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Additional tests for comprehensive coverage */
    
    /* Test 5: GT_EXPR with unsigned integers */
    printf("Testing GT_EXPR with unsigned integers...\n");
    test_gt_unsigned(dest_int, src1_uint, src2_uint, 77, -77);
    ref_gt_int(ref_int, (int*)src1_uint, (int*)src2_uint, 77, -77);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR unsigned test failed\n");
        errors++;
    }
    
    /* Test 6: GE_EXPR with signed integers (reduction) */
    printf("Testing GE_EXPR with signed integers (reduction)...\n");
    int vec_sum = test_ge_int_reduction(src1_int, src2_int);
    int ref_sum = 0;
    for (int i = 0; i < N; i++) {
        ref_sum += (src1_int[i] >= src2_int[i]);
    }
    if (vec_sum != ref_sum) {
        printf("  ERROR: GE_EXPR integer reduction test failed: %d vs %d\n",
               vec_sum, ref_sum);
        errors++;
    }
    
    /* Test 7: LT_EXPR with floats (mask generation) */
    printf("Testing LT_EXPR with floats (mask generation)...\n");
    test_lt_float_mask(mask_int, src1_float, src2_float);
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_float[i] < src2_float[i];
    }
    if (memcmp(mask_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR float mask test failed\n");
        errors++;
    }
    
    /* Test 8: LE_EXPR with unsigned integers */
    printf("Testing LE_EXPR with unsigned integers...\n");
    test_le_unsigned(dest_int, src1_uint, src2_uint, 88, -88);
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_uint[i] <= src2_uint[i] ? 88 : -88;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LE_EXPR unsigned test failed\n");
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
