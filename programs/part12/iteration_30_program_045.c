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
        count += (src1[i] < src2[i]);
    }
    return count;
}

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered patterns */

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
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* LE_EXPR with doubles - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for mixed patterns */

/* GT_EXPR with reduction pattern */
static int test_gt_reduction(const int *src1, const int *src2) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] > src2[i]);
    }
    escape(&count);
    return count;
}

/* GE_EXPR with mask generation for unsigned */
static void test_ge_unsigned_mask(unsigned *mask, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? 0xFFFFFFFF : 0;
    }
    escape(mask);
}

/* LT_EXPR with float conditional assignment */
static void test_lt_float_cond(float *dest, const float *src1, const float *src2, float threshold) {
    for (int i = 0; i < N; i++) {
        if (src1[i] < src2[i]) {
            dest[i] = src1[i] * threshold;
        } else {
            dest[i] = src2[i] * threshold;
        }
    }
    escape(dest);
}

/* LE_EXPR with integer mask and arithmetic */
static void test_le_int_mask(int *dest, const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        int mask = src1[i] <= src2[i];
        dest[i] = mask * src1[i] + (1 - mask) * src2[i];
    }
    escape(dest);
}

int main() {
    int result = 0;
    
    /* Aligned arrays for vector loads */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N], dest_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer patterns: alternating greater/lesser */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned patterns: different pattern for unsigned overflow cases */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        /* Float patterns: include special values and regular pattern */
        src1_float[i] = (i % 5 == 0) ? i * 1.5f : (i % 5 == 1) ? i * 0.5f : i * 1.0f;
        src2_float[i] = (i % 5 == 0) ? i * 1.0f : (i % 5 == 1) ? i * 0.75f : i * 1.25f;
        
        /* Double patterns: similar to float but with different scaling */
        src1_double[i] = (i % 7 == 0) ? i * 2.0 : (i % 7 == 1) ? i * 0.25 : i * 1.0;
        src2_double[i] = (i % 7 == 0) ? i * 1.5 : (i % 7 == 1) ? i * 0.5 : i * 1.1;
    }
    
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, 200);
    ref_gt_int(ref_int, src1_int, src2_int, 100, 200);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        result = 1;
    }
    
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        result = 1;
    }
    
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count_lt = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count_lt = ref_lt_unsigned(src1_uint, src2_uint);
    if (count_lt != ref_count_lt) {
        printf("FAIL: LT_EXPR unsigned test (got %u, expected %u)\n", count_lt, ref_count_lt);
        result = 1;
    }
    
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        result = 1;
    }
    
    /* Additional mixed pattern tests */
    printf("Testing GT_EXPR reduction pattern...\n");
    int gt_red = test_gt_reduction(src1_int, src2_int);
    int gt_red_ref = 0;
    for (int i = 0; i < N; i++) {
        gt_red_ref += (src1_int[i] > src2_int[i]);
    }
    if (gt_red != gt_red_ref) {
        printf("FAIL: GT_EXPR reduction test\n");
        result = 1;
    }
    
    printf("Testing GE_EXPR unsigned mask...\n");
    ALIGNED unsigned mask_uint[N];
    test_ge_unsigned_mask(mask_uint, src1_uint, src2_uint);
    for (int i = 0; i < N; i++) {
        unsigned expected = src1_uint[i] >= src2_uint[i] ? 0xFFFFFFFF : 0;
        if (mask_uint[i] != expected) {
            printf("FAIL: GE_EXPR mask test at index %d\n", i);
            result = 1;
            break;
        }
    }
    
    printf("Testing LT_EXPR float conditional...\n");
    ALIGNED float dest_float_cond[N];
    test_lt_float_cond(dest_float_cond, src1_float, src2_float, 2.0f);
    for (int i = 0; i < N; i++) {
        float expected = src1_float[i] < src2_float[i] ? src1_float[i] * 2.0f : src2_float[i] * 2.0f;
        if (dest_float_cond[i] != expected) {
            printf("FAIL: LT_EXPR float conditional test at index %d\n", i);
            result = 1;
            break;
        }
    }
    
    printf("Testing LE_EXPR integer mask arithmetic...\n");
    ALIGNED int dest_int_mask[N];
    test_le_int_mask(dest_int_mask, src1_int, src2_int);
    for (int i = 0; i < N; i++) {
        int mask = src1_int[i] <= src2_int[i];
        int expected = mask * src1_int[i] + (1 - mask) * src2_int[i];
        if (dest_int_mask[i] != expected) {
            printf("FAIL: LE_EXPR mask arithmetic test at index %d\n", i);
            result = 1;
            break;
        }
    }
    
    if (result == 0) {
        printf("All tests passed!\n");
    }
    
    return result;
}
