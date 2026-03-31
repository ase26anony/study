#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

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
        count += src1[i] < src2[i] ? 1 : 0;
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
        count += src1[i] < src2[i] ? 1 : 0;
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

static void test_le_float_swapped(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src2[i] <= src1[i] ? -1 : 0;  /* Equivalent to src1[i] >= src2[i] */
    }
    escape(mask);
}

/* Mixed pattern data generation */
static void init_data(int *int1, int *int2, 
                      unsigned *uint1, unsigned *uint2,
                      float *float1, float *float2,
                      double *double1, double *double2) {
    for (int i = 0; i < N; i++) {
        /* Create mixed comparison results */
        int1[i] = (i % 3 == 0) ? i + 10 : i - 5;
        int2[i] = i;
        
        uint1[i] = (i % 4 == 0) ? i + 7 : i - 3;
        uint2[i] = i;
        
        float1[i] = (i % 5 == 0) ? i * 1.5f : i * 0.8f;
        float2[i] = i * 1.0f;
        
        double1[i] = (i % 6 == 0) ? i * 2.0 : i * 0.7;
        double2[i] = i * 1.0;
    }
}

int main() {
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    ALIGNED int dest_int[N], ref_int[N], mask_int[N], ref_mask[N];
    ALIGNED double dest_double[N], ref_double[N];
    
    /* Initialize test data */
    init_data(src1_int, src2_int, src1_uint, src2_uint, 
              src1_float, src2_float, src1_double, src2_double);
    
    int errors = 0;
    
    /* Test 1: GT_EXPR with integers */
    test_gt_int(dest_int, src1_int, src2_int, 0xAAAA, 0x5555);
    ref_gt_int(ref_int, src1_int, src2_int, 0xAAAA, 0x5555);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        fprintf(stderr, "Error: GT_EXPR integer test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        fprintf(stderr, "Error: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    unsigned count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (count != ref_count) {
        fprintf(stderr, "Error: LT_EXPR unsigned reduction test failed: %u vs %u\n", 
                count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    for (int i = 0; i < N; i++) {
        if (fabs(dest_double[i] - ref_double[i]) > 1e-10) {
            fprintf(stderr, "Error: LE_EXPR double test failed at index %d: %f vs %f\n",
                    i, dest_double[i], ref_double[i]);
            errors++;
            break;
        }
    }
    
    /* Test 5: LT_EXPR with swapped operands (should trigger std::swap) */
    test_lt_int_swapped(dest_int, src1_int, src2_int, 0xAAAA, 0x5555);
    ref_gt_int(ref_int, src1_int, src2_int, 0xAAAA, 0x5555);  /* Same as GT test */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        fprintf(stderr, "Error: LT_EXPR swapped integer test failed\n");
        errors++;
    }
    
    /* Test 6: LE_EXPR with swapped operands (should trigger std::swap) */
    test_le_float_swapped(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);  /* Same as GE test */
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        fprintf(stderr, "Error: LE_EXPR swapped float test failed\n");
        errors++;
    }
    
    /* Additional edge cases */
    /* Test with all true comparisons */
    for (int i = 0; i < N; i++) {
        src1_int[i] = i + 1;  /* Always greater than src2 */
        src2_int[i] = i;
    }
    test_gt_int(dest_int, src1_int, src2_int, 1, 0);
    for (int i = 0; i < N; i++) {
        if (dest_int[i] != 1) {
            fprintf(stderr, "Error: All-true GT test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* Test with all false comparisons */
    for (int i = 0; i < N; i++) {
        src1_int[i] = i - 1;  /* Always less than src2 */
        src2_int[i] = i;
    }
    test_gt_int(dest_int, src1_int, src2_int, 1, 0);
    for (int i = 0; i < N; i++) {
        if (dest_int[i] != 0) {
            fprintf(stderr, "Error: All-false GT test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("%d test(s) failed\n", errors);
    }
    
    return errors;
}
