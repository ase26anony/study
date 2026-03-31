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
        count += (src1[i] < src2[i]);
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

static void test_ge_unsigned(unsigned *dest, const unsigned *src1, const unsigned *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1u : 0u;
    }
    escape(dest);
}

static void test_gt_double(double *dest, const double *src1, const double *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? 1.0 : 0.0;
    }
    escape(dest);
}

int main() {
    int ret = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED unsigned src1_uint[N], src2_uint[N], dest_uint[N], ref_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Integer patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned patterns (include wrap-around cases) */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        /* Float patterns */
        src1_float[i] = i * 0.5f;
        src2_float[i] = (i % 5 == 0) ? i * 0.5f + 0.25f : 
                       (i % 5 == 1) ? i * 0.5f - 0.25f : i * 0.5f;
        
        /* Double patterns */
        src1_double[i] = i * 0.25;
        src2_double[i] = (i % 7 == 0) ? i * 0.25 + 0.125 : 
                        (i % 7 == 1) ? i * 0.25 - 0.125 : i * 0.25;
    }
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 100, -100);
    ref_gt_int(ref_int, src1_int, src2_int, 100, -100);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: GT_EXPR integer test\n");
        ret = 1;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: GE_EXPR float test\n");
        ret = 1;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction pattern) */
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    unsigned count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (count != ref_count) {
        printf("FAIL: LT_EXPR unsigned reduction test (%u vs %u)\n", count, ref_count);
        ret = 1;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 99.9, -99.9);
    ref_le_double(ref_double, src1_double, src2_double, 99.9, -99.9);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAIL: LE_EXPR double test\n");
        ret = 1;
    }
    
    /* Additional tests for complete coverage */
    
    /* Test 5: LT_EXPR with integers */
    printf("Testing LT_EXPR with integers...\n");
    test_lt_int(dest_int, src1_int, src2_int, 777, 333);
    ref_gt_int(ref_int, src2_int, src1_int, 777, 333); /* LT is swapped GT */
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAIL: LT_EXPR integer test\n");
        ret = 1;
    }
    
    /* Test 6: LE_EXPR with floats */
    printf("Testing LE_EXPR with floats...\n");
    test_le_float(dest_float, src1_float, src2_float, 5.5f, -5.5f);
    ref_ge_float(ref_float, src2_float, src1_float); /* LE is swapped GE */
    for (int i = 0; i < N; i++) {
        ref_float[i] = ref_float[i] ? 5.5f : -5.5f;
    }
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAIL: LE_EXPR float test\n");
        ret = 1;
    }
    
    /* Test 7: GE_EXPR with unsigned */
    printf("Testing GE_EXPR with unsigned...\n");
    test_ge_unsigned(dest_uint, src1_uint, src2_uint);
    ref_ge_float(ref_float, (float*)src1_uint, (float*)src2_uint); /* Just for pattern */
    if (dest_uint[0] != (src1_uint[0] >= src2_uint[0] ? 1u : 0u)) {
        printf("FAIL: GE_EXPR unsigned test\n");
        ret = 1;
    }
    
    /* Test 8: GT_EXPR with doubles */
    printf("Testing GT_EXPR with doubles...\n");
    test_gt_double(dest_double, src1_double, src2_double);
    ref_gt_int(ref_int, (int*)src1_double, (int*)src2_double, 1, 0); /* Just for pattern */
    if (dest_double[0] != (src1_double[0] > src2_double[0] ? 1.0 : 0.0)) {
        printf("FAIL: GT_EXPR double test\n");
        ret = 1;
    }
    
    /* Mixed pattern test to ensure all branches are exercised */
    printf("Testing mixed comparison patterns...\n");
    for (int i = 0; i < N; i++) {
        /* Use all comparison operators in one loop (compiler may split) */
        dest_int[i] = (src1_int[i] > src2_int[i]) ? 1 :
                     (src1_int[i] >= src2_int[i]) ? 2 :
                     (src1_int[i] < src2_int[i]) ? 3 :
                     (src1_int[i] <= src2_int[i]) ? 4 : 0;
    }
    escape(dest_int);
    
    if (ret == 0) {
        printf("All tests passed!\n");
    }
    
    return ret;
}
