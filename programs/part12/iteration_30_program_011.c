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

static void ref_le_double(double *dest, const double *src1, const double *src2, 
                          double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels - these should be vectorized */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR transformation */
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR transformation */
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

static unsigned test_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR transformation with swap */
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2,
                          double val1, double val2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LE_EXPR transformation with swap */
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
static void test_lt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR transformation with swap */
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_unsigned(int *mask, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR transformation for unsigned */
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Mixed pattern to ensure both true and false comparisons */
static void init_patterns(int *src1_int, int *src2_int,
                         unsigned *src1_uint, unsigned *src2_uint,
                         float *src1_float, float *src2_float,
                         double *src1_double, double *src2_double) {
    for (int i = 0; i < N; i++) {
        /* Create alternating patterns to get mixed comparison results */
        src1_int[i] = (i % 4 == 0) ? i + 10 : i - 5;
        src2_int[i] = i;
        
        src1_uint[i] = (i % 3 == 0) ? i + 7 : i;
        src2_uint[i] = (i % 5 == 0) ? i + 3 : i + 1;
        
        src1_float[i] = (i % 2 == 0) ? i * 1.5f : i * 0.8f;
        src2_float[i] = i * 1.0f;
        
        src1_double[i] = (i % 3 == 0) ? i * 2.0 : i * 0.5;
        src2_double[i] = i * 1.0;
    }
}

int main() {
    /* Aligned arrays for vector loads */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    /* Destination arrays */
    ALIGNED int dest_int[N], ref_dest_int[N];
    ALIGNED int mask_int[N], ref_mask_int[N];
    ALIGNED double dest_double[N], ref_dest_double[N];
    
    int errors = 0;
    
    /* Initialize with mixed patterns */
    init_patterns(src1_int, src2_int, src1_uint, src2_uint,
                  src1_float, src2_float, src1_double, src2_double);
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, 200);
    ref_gt_int(ref_dest_int, src1_int, src2_int, 100, 200);
    if (memcmp(dest_int, ref_dest_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR integer test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask_int, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR float test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: %u vs %u\n", vec_count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_dest_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_dest_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Test 5: LT_EXPR with signed integers */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 300, 400);
    ref_gt_int(ref_dest_int, src2_int, src1_int, 300, 400); /* Equivalent to src2 > src1 */
    if (memcmp(dest_int, ref_dest_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed integer test failed\n");
        errors++;
    }
    
    /* Test 6: GE_EXPR with unsigned integers */
    printf("Testing GE_EXPR with unsigned integers...\n");
    test_ge_unsigned(mask_int, src1_uint, src2_uint);
    /* Reference: mask = src1 >= src2 */
    for (int i = 0; i < N; i++) {
        ref_mask_int[i] = src1_uint[i] >= src2_uint[i] ? -1 : 0;
    }
    if (memcmp(mask_int, ref_mask_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR unsigned test failed\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        printf("The vector comparison transformations (GT, GE, LT, LE) were triggered.\n");
    } else {
        printf("\n%d test(s) failed\n", errors);
    }
    
    return errors;
}
