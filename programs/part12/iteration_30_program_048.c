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

static int ref_lt_unsigned(const unsigned int *src1, const unsigned int *src2) {
    int count = 0;
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

/* Test kernels - these should be vectorized */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation */
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation */
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

static int test_lt_unsigned(const unsigned int *src1, const unsigned int *src2) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        /* This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */
static void test_lt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

static void test_le_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Test with mixed patterns to ensure both true and false comparisons */
static void init_test_data(ALIGNED int *src1_int, ALIGNED int *src2_int,
                           ALIGNED unsigned int *src1_uint, ALIGNED unsigned int *src2_uint,
                           ALIGNED float *src1_float, ALIGNED float *src2_float,
                           ALIGNED double *src1_double, ALIGNED double *src2_double) {
    for (int i = 0; i < N; i++) {
        /* Create patterns that yield mixed comparison results */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        src1_float[i] = (float)i * 1.5f;
        src2_float[i] = (float)((i % 5) - 2) * 2.0f + src1_float[i];
        
        src1_double[i] = (double)i * 0.75;
        src2_double[i] = (double)((i % 7) - 3) * 1.5 + src1_double[i];
    }
}

int main() {
    /* Aligned arrays for vector loads/stores */
    ALIGNED int src1_int[N], src2_int[N];
    ALIGNED unsigned int src1_uint[N], src2_uint[N];
    ALIGNED float src1_float[N], src2_float[N];
    ALIGNED double src1_double[N], src2_double[N];
    
    /* Destination arrays */
    ALIGNED int dest_int[N], ref_int[N];
    ALIGNED float dest_float[N], ref_float[N];
    ALIGNED double dest_double[N], ref_double[N];
    
    /* Initialize test data with mixed patterns */
    init_test_data(src1_int, src2_int, src1_uint, src2_uint,
                   src1_float, src2_float, src1_double, src2_double);
    
    int errors = 0;
    
    /* Test 1: GT comparison with integers */
    printf("Testing GT with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT integer test failed\n");
        errors++;
    }
    
    /* Test 2: GE comparison with floats */
    printf("Testing GE with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("  ERROR: GE float test failed\n");
        errors++;
    }
    
    /* Test 3: LT comparison with unsigned integers (reduction pattern) */
    printf("Testing LT with unsigned integers (reduction)...\n");
    int count = test_lt_unsigned(src1_uint, src2_uint);
    int ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (count != ref_count) {
        printf("  ERROR: LT unsigned reduction test failed: %d != %d\n", count, ref_count);
        errors++;
    }
    
    /* Test 4: LE comparison with doubles */
    printf("Testing LE with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, -2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, -2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE double test failed\n");
        errors++;
    }
    
    /* Additional tests for completeness */
    printf("Testing LT with integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 77, -77);
    ref_gt_int(ref_int, src2_int, src1_int, 77, -77); /* LT(a,b) == GT(b,a) */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LT integer test failed\n");
        errors++;
    }
    
    printf("Testing LE with integers...\n");
    test_le_int(dest_int, src1_int, src2_int, 88, -88);
    ref_ge_int(ref_int, src2_int, src1_int, 88, -88); /* LE(a,b) == GE(b,a) */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: LE integer test failed\n");
        errors++;
    }
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        printf("The vector comparison transformations (GT, GE, LT, LE) should have been triggered.\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}

/* Helper reference function for LE test */
static void ref_ge_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
}
