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

/* Test kernels - should be vectorized */
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

/* Additional tests for LT/LE with swapped operands */
static void test_lt_int_swapped(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src2[i] < src1[i] ? val1 : val2;  /* Equivalent to src1[i] > src2[i] */
    }
    escape(dest);
}

static void test_le_float_swapped(float *dest, const float *src1, const float *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src2[i] <= src1[i] ? val1 : val2;  /* Equivalent to src1[i] >= src2[i] */
    }
    escape(dest);
}

/* Mixed pattern data generation */
static void init_data(int *int1, int *int2, unsigned *uint1, unsigned *uint2,
                      float *float1, float *float2, double *double1, double *double2) {
    for (int i = 0; i < N; i++) {
        /* Create mixed comparison results */
        int1[i] = i;
        int2[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        uint1[i] = i * 2;
        uint2[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        float1[i] = (i % 5) * 1.5f;
        float2[i] = (i % 5 == 0) ? float1[i] + 0.5f : 
                   (i % 5 == 1) ? float1[i] - 0.5f : float1[i];
        
        double1[i] = (i % 7) * 2.3;
        double2[i] = (i % 7 == 0) ? double1[i] + 1.0 :
                    (i % 7 == 1) ? double1[i] - 1.0 : double1[i];
    }
}

int main() {
    /* Aligned arrays for vector loads */
    ALIGNED int int_src1[N], int_src2[N], int_dest[N], int_ref[N];
    ALIGNED unsigned uint_src1[N], uint_src2[N];
    ALIGNED float float_src1[N], float_src2[N], float_dest[N], float_ref[N];
    ALIGNED double double_src1[N], double_src2[N], double_dest[N], double_ref[N];
    
    int errors = 0;
    
    /* Initialize with mixed pattern data */
    init_data(int_src1, int_src2, uint_src1, uint_src2,
              float_src1, float_src2, double_src1, double_src2);
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    ref_gt_int(int_ref, int_src1, int_src2, 99, -99);
    test_gt_int(int_dest, int_src1, int_src2, 99, -99);
    
    if (memcmp(int_ref, int_dest, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    ref_ge_float(float_ref, float_src1, float_src2);
    test_ge_float(float_dest, float_src1, float_src2);
    
    if (memcmp(float_ref, float_dest, N * sizeof(float)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned ref_count = ref_lt_unsigned(uint_src1, uint_src2);
    unsigned test_count = test_lt_unsigned(uint_src1, uint_src2);
    
    if (ref_count != test_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed: ref=%u, test=%u\n",
               ref_count, test_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    ref_le_double(double_ref, double_src1, double_src2, 3.14159, 2.71828);
    test_le_double(double_dest, double_src1, double_src2, 3.14159, 2.71828);
    
    if (memcmp(double_ref, double_dest, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Test 5: LT_EXPR with swapped operands (should trigger std::swap) */
    printf("Testing LT_EXPR with swapped operands...\n");
    ref_gt_int(int_ref, int_src1, int_src2, 77, 88);  /* Same as test_gt_int */
    test_lt_int_swapped(int_dest, int_src1, int_src2, 77, 88);
    
    if (memcmp(int_ref, int_dest, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR swapped test failed\n");
        errors++;
    }
    
    /* Test 6: LE_EXPR with swapped operands (should trigger std::swap) */
    printf("Testing LE_EXPR with swapped operands...\n");
    ref_ge_float(float_ref, float_src1, float_src2);
    test_le_float_swapped(float_dest, float_src1, float_src2, 1.0f, 0.0f);
    
    if (memcmp(float_ref, float_dest, N * sizeof(float)) != 0) {
        printf("  ERROR: LE_EXPR swapped test failed\n");
        errors++;
    }
    
    /* Additional edge cases */
    printf("Testing edge cases...\n");
    
    /* Test with all true comparisons */
    for (int i = 0; i < N; i++) {
        int_src1[i] = i + 1;
        int_src2[i] = i;
    }
    test_gt_int(int_dest, int_src1, int_src2, 1, 0);
    ref_gt_int(int_ref, int_src1, int_src2, 1, 0);
    
    if (memcmp(int_ref, int_dest, N * sizeof(int)) != 0) {
        printf("  ERROR: All-true GT_EXPR test failed\n");
        errors++;
    }
    
    /* Test with all false comparisons */
    for (int i = 0; i < N; i++) {
        int_src1[i] = i;
        int_src2[i] = i + 1;
    }
    test_gt_int(int_dest, int_src1, int_src2, 1, 0);
    ref_gt_int(int_ref, int_src1, int_src2, 1, 0);
    
    if (memcmp(int_ref, int_dest, N * sizeof(int)) != 0) {
        printf("  ERROR: All-false GT_EXPR test failed\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
