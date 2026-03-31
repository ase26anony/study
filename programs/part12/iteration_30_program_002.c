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
static void ref_gt_int(int *dest, int *src1, int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void ref_ge_float(float *dest, float *src1, float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

static unsigned ref_lt_unsigned(unsigned *src1, unsigned *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void ref_le_double(double *dest, double *src1, double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Test kernels targeting specific uncovered transformations */

/* GT_EXPR transformation for integers */
static void test_gt_int(int ALIGNED *dest, int ALIGNED *src1, int ALIGNED *src2, 
                        int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR transformation for floats */
static void test_ge_float(float ALIGNED *dest, float ALIGNED *src1, float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* LT_EXPR transformation for unsigned integers (with potential swap) */
static unsigned test_lt_unsigned(unsigned ALIGNED *src1, unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* LE_EXPR transformation for doubles (with potential swap) */
static void test_le_double(double ALIGNED *dest, double ALIGNED *src1, double ALIGNED *src2,
                          double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
static void test_lt_int(int ALIGNED *dest, int ALIGNED *src1, int ALIGNED *src2,
                       int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_unsigned(unsigned ALIGNED *dest, unsigned ALIGNED *src1, 
                            unsigned ALIGNED *src2, unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Mixed pattern to ensure various comparison results */
static void init_patterns(int *src1_int, int *src2_int,
                         unsigned *src1_uint, unsigned *src2_uint,
                         float *src1_float, float *src2_float,
                         double *src1_double, double *src2_double) {
    for (int i = 0; i < N; i++) {
        /* Create alternating patterns for different comparison outcomes */
        src1_int[i] = (i % 4 == 0) ? i + 10 : i - 5;
        src2_int[i] = i;
        
        src1_uint[i] = (i % 3 == 0) ? i * 2 : i / 2;
        src2_uint[i] = i;
        
        src1_float[i] = (i % 5 == 0) ? i * 1.5f : i * 0.8f;
        src2_float[i] = i * 1.0f;
        
        src1_double[i] = (i % 6 == 0) ? i * 2.0 : i * 0.5;
        src2_double[i] = i * 1.0;
    }
}

int main() {
    /* Aligned arrays for vectorization */
    int ALIGNED src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    unsigned ALIGNED src1_uint[N], src2_uint[N], dest_uint[N], ref_uint[N];
    float ALIGNED src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    double ALIGNED src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    int errors = 0;
    
    /* Initialize with mixed patterns */
    init_patterns(src1_int, src2_int, src1_uint, src2_uint,
                  src1_float, src2_float, src1_double, src2_double);
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR (int)...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, -100);
    ref_gt_int(ref_int, src1_int, src2_int, 100, -100);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR int test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR (float)...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    for (int i = 0; i < N; i++) {
        if (fabs(dest_float[i] - ref_float[i]) > 0.0001f) {
            printf("  ERROR: GE_EXPR float test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* Test 3: LT_EXPR with unsigned integers */
    printf("Testing LT_EXPR (unsigned)...\n");
    unsigned count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: %u vs %u\n", count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR (double)...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    for (int i = 0; i < N; i++) {
        if (fabs(dest_double[i] - ref_double[i]) > 0.000001) {
            printf("  ERROR: LE_EXPR double test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* Additional tests for completeness */
    
    /* Test 5: LT_EXPR with signed integers */
    printf("Testing LT_EXPR (int)...\n");
    test_lt_int(dest_int, src1_int, src2_int, 777, 999);
    ref_gt_int(ref_int, src2_int, src1_int, 777, 999); /* Equivalent to src2 > src1 */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR int test failed\n");
        errors++;
    }
    
    /* Test 6: GE_EXPR with unsigned integers */
    printf("Testing GE_EXPR (unsigned)...\n");
    test_ge_unsigned(dest_uint, src1_uint, src2_uint, 0xFFFFFFFF, 0x00000000);
    /* Reference implementation */
    for (int i = 0; i < N; i++) {
        ref_uint[i] = src1_uint[i] >= src2_uint[i] ? 0xFFFFFFFF : 0x00000000;
    }
    if (memcmp(dest_uint, ref_uint, N * sizeof(unsigned)) != 0) {
        printf("  ERROR: GE_EXPR unsigned test failed\n");
        errors++;
    }
    
    /* Summary */
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        printf("The uncovered comparison transformations (GT, GE, LT, LE) should have been triggered.\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
