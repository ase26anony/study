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

/* Reference implementations for verification */
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

/* Test kernels - should trigger vectorization transformations */
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

/* Mixed pattern tests */
static void test_mixed_comparisons(int *results, const int *a, const int *b, const float *fa, const float *fb) {
    for (int i = 0; i < N; i++) {
        /* Mix different comparison types */
        int r1 = a[i] > b[i] ? 1 : 0;
        int r2 = a[i] >= b[i] ? 2 : 0;
        int r3 = a[i] < b[i] ? 4 : 0;
        int r4 = a[i] <= b[i] ? 8 : 0;
        int r5 = fa[i] > fb[i] ? 16 : 0;
        int r6 = fa[i] >= fb[i] ? 32 : 0;
        int r7 = fa[i] < fb[i] ? 64 : 0;
        int r8 = fa[i] <= fb[i] ? 128 : 0;
        
        results[i] = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8;
    }
    escape(results);
}

int main() {
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    ALIGNED int mixed_results[N];
    
    /* Initialize with pattern that creates mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer arrays: alternating patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned arrays: different pattern */
        src1_uint[i] = i * 2;
        src2_uint[i] = i * 2 + (i % 5);
        
        /* Float arrays: create some NaN/inf values for edge cases */
        src1_float[i] = (i % 7 == 0) ? (float)i * 1.5f : (float)i;
        src2_float[i] = (i % 7 == 1) ? (float)i * 0.5f : (float)(i + 1);
        
        /* Double arrays: similar pattern */
        src1_double[i] = (i % 11 == 0) ? (double)i * 2.0 : (double)i;
        src2_double[i] = (i % 11 == 1) ? (double)i * 0.5 : (double)(i + 2);
    }
    
    int errors = 0;
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 777, 999);
    ref_gt_int(ref_int, src1_int, src2_int, 777, 999);
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
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR unsigned test failed: %u != %u\n", vec_count, ref_count);
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
    
    /* Test 5: LT_EXPR with swapped operands (should trigger std::swap) */
    printf("Testing LT_EXPR with swapped operands...\n");
    test_lt_int_swapped(dest_int, src1_int, src2_int, 111, 222);
    ref_gt_int(ref_int, src1_int, src2_int, 111, 222);  /* Same as GT */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR swapped test failed\n");
        errors++;
    }
    
    /* Test 6: LE_EXPR with swapped operands (should trigger std::swap) */
    printf("Testing LE_EXPR with swapped operands...\n");
    test_le_float_swapped(dest_float, src1_float, src2_float, 5.5f, 6.6f);
    ref_ge_float(ref_float, src1_float, src2_float);  /* Convert to 1.0/0.0 for comparison */
    for (int i = 0; i < N; i++) {
        float expected = ref_float[i] > 0.5f ? 5.5f : 6.6f;
        if (dest_float[i] != expected) {
            printf("  ERROR: LE_EXPR swapped test failed at index %d: %f != %f\n", 
                   i, dest_float[i], expected);
            errors++;
            break;
        }
    }
    
    /* Test 7: Mixed comparisons in one loop */
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(mixed_results, src1_int, src2_int, src1_float, src2_float);
    escape(mixed_results);
    
    if (errors == 0) {
        printf("\nAll tests passed!\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
