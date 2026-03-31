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

/* Test kernels that should trigger vectorization transformations */

/* Test 1: GT_EXPR with integers - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

/* Test 2: GE_EXPR with floats - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

/* Test 3: LT_EXPR with unsigned integers - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR with swap */
static uint32_t test_lt_unsigned(const uint32_t *src1, const uint32_t *src2) {
    uint32_t count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

/* Test 4: LE_EXPR with doubles - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR with swap */
static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test 5: Mixed comparisons in same loop to trigger multiple transformations */
static void test_mixed_comparisons(int *dest_int, float *dest_float, 
                                   const int *src1_int, const int *src2_int,
                                   const float *src1_float, const float *src2_float) {
    for (int i = 0; i < N; i++) {
        /* GT and LE in same loop */
        dest_int[i] = src1_int[i] > src2_int[i] ? 100 : -100;
        dest_float[i] = src1_float[i] <= src2_float[i] ? 2.5f : -2.5f;
    }
}

/* Test 6: Conditional assignment with GE and LT */
static void test_conditional_assign(int *dest, const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        if (src1[i] >= src2[i]) {
            dest[i] = src1[i] * 2;
        } else if (src1[i] < src2[i]) {
            dest[i] = src1[i] / 2;
        } else {
            dest[i] = 0;
        }
    }
}

/* Test 7: Mask generation with different comparison types */
static void test_mask_generation(uint8_t *mask_gt, uint8_t *mask_ge, 
                                 uint8_t *mask_lt, uint8_t *mask_le,
                                 const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        mask_gt[i] = src1[i] > src2[i];
        mask_ge[i] = src1[i] >= src2[i];
        mask_lt[i] = src1[i] < src2[i];
        mask_le[i] = src1[i] <= src2[i];
    }
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED uint32_t src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    ALIGNED uint8_t mask_gt[N], mask_ge[N], mask_lt[N], mask_le[N];
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : 
                     (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : i * 2 - 1;
        
        /* Float arrays: create some NaN/inf patterns */
        src1_float[i] = (i % 5 == 0) ? (float)i * 1.5f : 
                       (i % 5 == 1) ? (float)i * 0.5f :
                       (i % 5 == 2) ? -(float)i : 
                       (i % 5 == 3) ? 0.0f : (float)i;
        src2_float[i] = (i % 7 == 0) ? (float)i * 1.3f : 
                       (i % 7 == 1) ? (float)i * 0.7f :
                       (i % 7 == 2) ? -(float)i * 2.0f : 
                       (i % 7 == 3) ? -0.0f : (float)i + 10.0f;
        
        /* Double arrays: similar pattern */
        src1_double[i] = (double)i * 1.1;
        src2_double[i] = (double)(i % 3) * 0.9;
    }
    
    /* Prevent optimization of initialization */
    escape(src1_int); escape(src2_int); escape(src1_uint); escape(src2_uint);
    escape(src1_float); escape(src2_float); escape(src1_double); escape(src2_double);
    
    printf("Testing vector comparison transformations...\n");
    
    /* Test 1: GT_EXPR with integers */
    printf("Test 1: GT_EXPR with integers... ");
    test_gt_int(dest_int, src1_int, src2_int, 999, -999);
    ref_gt_int(ref_int, src1_int, src2_int, 999, -999);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Test 2: GE_EXPR with floats... ");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 3: LT_EXPR with unsigned integers */
    printf("Test 3: LT_EXPR with unsigned integers... ");
    uint32_t vec_count = test_lt_unsigned(src1_uint, src2_uint);
    uint32_t ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("FAILED (vec=%u, ref=%u)\n", vec_count, ref_count);
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Test 4: LE_EXPR with doubles... ");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, -3.14159);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, -3.14159);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 5: Mixed comparisons */
    printf("Test 5: Mixed comparisons in same loop... ");
    ALIGNED int dest_mixed_int[N];
    ALIGNED float dest_mixed_float[N];
    ALIGNED int ref_mixed_int[N];
    ALIGNED float ref_mixed_float[N];
    
    for (int i = 0; i < N; i++) {
        ref_mixed_int[i] = src1_int[i] > src2_int[i] ? 100 : -100;
        ref_mixed_float[i] = src1_float[i] <= src2_float[i] ? 2.5f : -2.5f;
    }
    
    test_mixed_comparisons(dest_mixed_int, dest_mixed_float, 
                          src1_int, src2_int, src1_float, src2_float);
    
    if (memcmp(dest_mixed_int, ref_mixed_int, N * sizeof(int)) != 0 ||
        memcmp(dest_mixed_float, ref_mixed_float, N * sizeof(float)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 6: Conditional assignment */
    printf("Test 6: Conditional assignment with GE and LT... ");
    ALIGNED int dest_cond[N];
    ALIGNED int ref_cond[N];
    
    test_conditional_assign(dest_cond, src1_int, src2_int);
    
    for (int i = 0; i < N; i++) {
        if (src1_int[i] >= src2_int[i]) {
            ref_cond[i] = src1_int[i] * 2;
        } else if (src1_int[i] < src2_int[i]) {
            ref_cond[i] = src1_int[i] / 2;
        } else {
            ref_cond[i] = 0;
        }
    }
    
    if (memcmp(dest_cond, ref_cond, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 7: Mask generation */
    printf("Test 7: Mask generation with all comparison types... ");
    test_mask_generation(mask_gt, mask_ge, mask_lt, mask_le, src1_int, src2_int);
    
    int mask_errors = 0;
    for (int i = 0; i < N; i++) {
        if (mask_gt[i] != (src1_int[i] > src2_int[i])) mask_errors++;
        if (mask_ge[i] != (src1_int[i] >= src2_int[i])) mask_errors++;
        if (mask_lt[i] != (src1_int[i] < src2_int[i])) mask_errors++;
        if (mask_le[i] != (src1_int[i] <= src2_int[i])) mask_errors++;
    }
    
    if (mask_errors > 0) {
        printf("FAILED (%d errors)\n", mask_errors);
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    printf("\nTotal errors: %d\n", errors);
    return errors > 0 ? 1 : 0;
}
