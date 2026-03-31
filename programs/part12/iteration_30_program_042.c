#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization removal */
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

/* Test kernels targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Case 3: LT_EXPR with unsigned integers - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional test cases for mixed patterns */

/* GT_EXPR with floating-point reduction */
static float test_gt_float_reduction(const float *src1, const float *src2, float threshold) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        if (src1[i] > src2[i]) {
            sum += threshold;
        }
    }
    escape(&sum);
    return sum;
}

/* LE_EXPR with signed integers in conditional assignment */
static void test_le_int_conditional(int *dest, const int *src1, const int *src2, int scale) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? src1[i] * scale : src2[i];
    }
    escape(dest);
}

/* GE_EXPR with unsigned mask generation */
static void test_ge_unsigned_mask(unsigned *mask, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? 0xFFFFFFFF : 0;
    }
    escape(mask);
}

/* LT_EXPR with double precision floating point */
static void test_lt_double_mask(int64_t *mask, const double *src1, const double *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] < src2[i] ? -1 : 0;
    }
    escape(mask);
}

int main() {
    int i;
    int errors = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N], mask_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    ALIGNED int64_t mask_int64[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned patterns */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        /* Float patterns */
        src1_float[i] = i * 0.5f;
        src2_float[i] = (i % 5 == 0) ? i * 0.5f + 0.1f : (i % 5 == 1) ? i * 0.5f - 0.1f : i * 0.5f;
        
        /* Double patterns */
        src1_double[i] = i * 0.25;
        src2_double[i] = (i % 7 == 0) ? i * 0.25 + 0.05 : (i % 7 == 1) ? i * 0.25 - 0.05 : i * 0.25;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, 200);
    ref_gt_int(ref_int, src1_int, src2_int, 100, 200);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_int, src1_float, src2_float);
    if (memcmp(mask_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count1 = test_lt_unsigned(src1_uint, src2_uint);
    unsigned count2 = ref_lt_unsigned(src1_uint, src2_uint);
    if (count1 != count2) {
        printf("  ERROR: LT_EXPR unsigned test failed: %u != %u\n", count1, count2);
        errors++;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 1.0, 0.0);
    ref_le_double(ref_double, src1_double, src2_double, 1.0, 0.0);
    for (i = 0; i < N; i++) {
        if (dest_double[i] != ref_double[i]) {
            printf("  ERROR: LE_EXPR double test failed at index %d: %f != %f\n", 
                   i, dest_double[i], ref_double[i]);
            errors++;
            break;
        }
    }
    
    /* Additional mixed tests */
    printf("Testing GT_EXPR with float reduction...\n");
    float sum1 = test_gt_float_reduction(src1_float, src2_float, 2.5f);
    float sum2 = 0.0f;
    for (i = 0; i < N; i++) {
        if (src1_float[i] > src2_float[i]) {
            sum2 += 2.5f;
        }
    }
    if (sum1 != sum2) {
        printf("  ERROR: GT_EXPR float reduction failed: %f != %f\n", sum1, sum2);
        errors++;
    }
    
    printf("Testing LE_EXPR with integer conditional...\n");
    test_le_int_conditional(dest_int, src1_int, src2_int, 3);
    for (i = 0; i < N; i++) {
        int expected = src1_int[i] <= src2_int[i] ? src1_int[i] * 3 : src2_int[i];
        if (dest_int[i] != expected) {
            printf("  ERROR: LE_EXPR int conditional failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    printf("Testing GE_EXPR with unsigned mask...\n");
    test_ge_unsigned_mask((unsigned*)mask_int, src1_uint, src2_uint);
    for (i = 0; i < N; i++) {
        unsigned expected = src1_uint[i] >= src2_uint[i] ? 0xFFFFFFFF : 0;
        if (((unsigned*)mask_int)[i] != expected) {
            printf("  ERROR: GE_EXPR unsigned mask failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    printf("Testing LT_EXPR with double mask...\n");
    test_lt_double_mask(mask_int64, src1_double, src2_double);
    for (i = 0; i < N; i++) {
        int64_t expected = src1_double[i] < src2_double[i] ? -1 : 0;
        if (mask_int64[i] != expected) {
            printf("  ERROR: LT_EXPR double mask failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        printf("The uncovered lines for GT, GE, LT, and LE comparisons should be triggered.\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
