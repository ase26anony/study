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
        count += (src1[i] < src2[i]) ? 1 : 0;
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

static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
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

/* Additional test cases for all comparison operators */
static void test_lt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_float(float *dest, const float *src1, const float *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

static unsigned test_gt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] > src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

static void test_ge_double(double *dest, const double *src1, const double *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0 : 0.0;
    }
    escape(dest);
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: create some NaN/inf values for edge cases */
        src1_float[i] = (i % 5 == 0) ? (float)(i) * 1.5f : (float)(i);
        src2_float[i] = (i % 5 == 1) ? (float)(i) * 1.5f : (float)(i + 1);
        
        /* Double arrays */
        src1_double[i] = (i % 7 == 0) ? (double)(i) * 2.0 : (double)(i);
        src2_double[i] = (i % 7 == 1) ? (double)(i) * 2.0 : (double)(i + 2);
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
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned test_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (test_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed: %u vs %u\n", 
               test_count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Additional tests for completeness */
    
    /* Test 5: LT_EXPR with integers */
    printf("Testing LT_EXPR with integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 777, 888);
    ref_gt_int(ref_int, src2_int, src1_int, 777, 888); /* Equivalent to src1 < src2 */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR integer test failed\n");
        errors++;
    }
    
    /* Test 6: LE_EXPR with floats */
    printf("Testing LE_EXPR with floats...\n");
    test_le_float(dest_float, src1_float, src2_float, 5.0f, 10.0f);
    ref_ge_float(ref_float, src2_float, src1_float); /* Equivalent to src1 <= src2 */
    for (int i = 0; i < N; i++) {
        float expected = ref_float[i] ? 5.0f : 10.0f;
        if (dest_float[i] != expected) {
            printf("  ERROR: LE_EXPR float test failed at index %d: %f vs %f\n",
                   i, dest_float[i], expected);
            errors++;
            break;
        }
    }
    
    /* Test 7: GT_EXPR with unsigned (reduction) */
    printf("Testing GT_EXPR with unsigned integers (reduction)...\n");
    test_count = test_gt_unsigned(src1_uint, src2_uint);
    ref_count = 0;
    for (int i = 0; i < N; i++) {
        ref_count += (src1_uint[i] > src2_uint[i]) ? 1 : 0;
    }
    if (test_count != ref_count) {
        printf("  ERROR: GT_EXPR unsigned reduction test failed: %u vs %u\n",
               test_count, ref_count);
        errors++;
    }
    
    /* Test 8: GE_EXPR with doubles */
    printf("Testing GE_EXPR with doubles...\n");
    test_ge_double(dest_double, src1_double, src2_double);
    ref_le_double(ref_double, src2_double, src1_double, 1.0, 0.0); /* Equivalent to src1 >= src2 */
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: GE_EXPR double test failed\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
