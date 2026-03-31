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

/* Vectorizable test kernels */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

static uint32_t test_lt_unsigned(const uint32_t *src1, const uint32_t *src2) {
    uint32_t count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Additional tests for signed/unsigned variations */
static void test_gt_unsigned(uint32_t *dest, const uint32_t *src1, const uint32_t *src2, uint32_t val1, uint32_t val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void test_lt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
}

static void test_ge_unsigned(uint32_t *dest, const uint32_t *src1, const uint32_t *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1 : 0;
    }
}

static void test_le_float(float *dest, const float *src1, const float *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED uint32_t src1_uint[N], src2_uint[N], dest_uint[N], ref_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer patterns: alternating relationships */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 :  /* src1 < src2 */
                      (i % 3 == 1) ? i - 1 :  /* src1 > src2 */
                                     i;       /* src1 == src2 */
        
        /* Unsigned patterns with wrap-around cases */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? UINT32_MAX - i : i * 3;
        
        /* Float patterns with NaN-free comparisons */
        src1_float[i] = (i % 5) * 1.5f;
        src2_float[i] = (i % 5 == 0) ? src1_float[i] + 0.1f :
                       (i % 5 == 1) ? src1_float[i] - 0.1f :
                       (i % 5 == 2) ? src1_float[i] :
                       (i % 5 == 3) ? -src1_float[i] :
                                      src1_float[i] * 2.0f;
        
        /* Double patterns similar to float */
        src1_double[i] = (i % 7) * 2.5;
        src2_double[i] = (i % 7 == 0) ? src1_double[i] + 0.01 :
                        (i % 7 == 1) ? src1_double[i] - 0.01 :
                        (i % 7 == 2) ? src1_double[i] :
                        (i % 7 == 3) ? -src1_double[i] :
                        (i % 7 == 4) ? src1_double[i] * 3.0 :
                        (i % 7 == 5) ? src1_double[i] / 2.0 :
                                      src1_double[i] + 100.0;
    }
    
    /* Test 1: GT_EXPR with signed integers */
    printf("Testing GT_EXPR with signed integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR int test failed\n");
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
    
    /* Test 3: LT_EXPR with unsigned integers (reduction pattern) */
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    uint32_t vec_count = test_lt_unsigned(src1_uint, src2_uint);
    uint32_t ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed: vec=%u, ref=%u\n", 
               vec_count, ref_count);
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
    
    /* Test 5: GT_EXPR with unsigned integers */
    printf("Testing GT_EXPR with unsigned integers...\n");
    test_gt_unsigned(dest_uint, src1_uint, src2_uint, 0xFFFFFFFF, 0x00000000);
    /* Reference implementation for unsigned GT */
    for (int i = 0; i < N; i++) {
        ref_uint[i] = src1_uint[i] > src2_uint[i] ? 0xFFFFFFFF : 0x00000000;
    }
    if (memcmp(dest_uint, ref_uint, N * sizeof(uint32_t)) != 0) {
        printf("  ERROR: GT_EXPR unsigned test failed\n");
        errors++;
    }
    
    /* Test 6: LT_EXPR with signed integers */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 777, 333);
    /* Reference implementation for signed LT */
    for (int i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] < src2_int[i] ? 777 : 333;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed test failed\n");
        errors++;
    }
    
    /* Test 7: GE_EXPR with unsigned integers */
    printf("Testing GE_EXPR with unsigned integers...\n");
    test_ge_unsigned(dest_uint, src1_uint, src2_uint);
    /* Reference implementation for unsigned GE */
    for (int i = 0; i < N; i++) {
        ref_uint[i] = src1_uint[i] >= src2_uint[i] ? 1 : 0;
    }
    if (memcmp(dest_uint, ref_uint, N * sizeof(uint32_t)) != 0) {
        printf("  ERROR: GE_EXPR unsigned test failed\n");
        errors++;
    }
    
    /* Test 8: LE_EXPR with floats */
    printf("Testing LE_EXPR with floats...\n");
    test_le_float(dest_float, src1_float, src2_float, 10.0f, -10.0f);
    /* Reference implementation for float LE */
    for (int i = 0; i < N; i++) {
        ref_float[i] = src1_float[i] <= src2_float[i] ? 10.0f : -10.0f;
    }
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("  ERROR: LE_EXPR float test failed\n");
        errors++;
    }
    
    /* Prevent dead code elimination */
    escape(dest_int);
    escape(dest_uint);
    escape(dest_float);
    escape(dest_double);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        return 0;
    } else {
        printf("\n%d test(s) failed!\n", errors);
        return 1;
    }
}
