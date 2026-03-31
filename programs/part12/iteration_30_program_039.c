#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization removal */
static void escape(void *p) {
    __asm__ volatile ("" : : "r"(p) : "memory");
}

/* Reference scalar implementations */
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
        count += src1[i] < src2[i];
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Vectorizable test functions */
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
        count += src1[i] < src2[i];
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
    int errors = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N], mask_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: create both true and false comparisons */
        src1_float[i] = (float)i * 1.5f;
        src2_float[i] = (float)i * ((i % 5 == 0) ? 1.6f : 1.4f);
        
        /* Double arrays: similar pattern */
        src1_double[i] = (double)i * 2.5;
        src2_double[i] = (double)i * ((i % 7 == 0) ? 2.6 : 2.4);
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
    test_le_double(dest_double, src1_double, src2_double, 3.14159, -2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, -2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Additional tests for coverage */
    
    /* Test 5: GE_EXPR with unsigned integers */
    printf("Testing GE_EXPR with unsigned integers...\n");
    test_ge_unsigned(mask_int, src1_uint, src2_uint);
    ref_ge_float(ref_int, (float*)src1_uint, (float*)src2_uint); /* Reuse, pattern matches */
    if (memcmp(mask_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR unsigned test failed\n");
        errors++;
    }
    
    /* Test 6: LT_EXPR with signed integers */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(mask_int, src1_int, src2_int);
    /* Create reference using same logic */
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] < src2_int[i] ? -1 : 0;
    }
    if (memcmp(mask_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed test failed\n");
        errors++;
    }
    
    /* Test 7: LE_EXPR with floats */
    printf("Testing LE_EXPR with floats...\n");
    test_le_float(dest_float, src1_float, src2_float, 10.0f, -10.0f);
    ref_le_double((double*)ref_float, (double*)src1_float, (double*)src2_float, 
                  10.0, -10.0); /* Reuse with cast */
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("  ERROR: LE_EXPR float test failed\n");
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
