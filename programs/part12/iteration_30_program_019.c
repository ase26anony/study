#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent optimization */
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

/* Test kernels - should be vectorized */
static void test_gt_int(int *dest, const int *src1, const int *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        /* GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation */
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
}

static void test_ge_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        /* GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation */
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
}

static uint32_t test_lt_unsigned(const uint32_t *src1, const uint32_t *src2) {
    uint32_t count = 0;
    for (int i = 0; i < N; i++) {
        /* LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR with swap */
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    return count;
}

static void test_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        /* LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Additional test cases for signed/unsigned variations */
static void test_gt_unsigned(uint32_t *dest, const uint32_t *src1, const uint32_t *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? 0xFFFFFFFF : 0;
    }
}

static void test_le_int(int *dest, const int *src1, const int *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? -1 : 0;
    }
}

static void test_ge_double(double *dest, const double *src1, const double *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 2.0 : -2.0;
    }
}

static void test_lt_float(float *dest, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? 3.14f : -3.14f;
    }
}

int main() {
    int errors = 0;
    
    /* Aligned arrays for vectorization */
    ALIGNED int src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    ALIGNED uint32_t src1_uint[N], src2_uint[N], dest_uint[N], ref_uint[N];
    ALIGNED float src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    ALIGNED double src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    
    /* Initialize with patterned data to create mixed comparison results */
    for (int i = 0; i < N; i++) {
        /* Pattern: alternating greater/lesser relationships */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 5 : (i % 4 == 1) ? i * 2 - 3 : i * 2;
        
        src1_float[i] = i * 0.5f;
        src2_float[i] = (i % 5 == 0) ? i * 0.5f + 0.7f : (i % 5 == 1) ? i * 0.5f - 0.3f : i * 0.5f;
        
        src1_double[i] = i * 0.25;
        src2_double[i] = (i % 6 == 0) ? i * 0.25 + 0.9 : (i % 6 == 1) ? i * 0.25 - 0.4 : i * 0.25;
    }
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
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
    printf("Testing LT_EXPR with unsigned integers (reduction)...\n");
    uint32_t count_vec = test_lt_unsigned(src1_uint, src2_uint);
    uint32_t count_ref = ref_lt_unsigned(src1_uint, src2_uint);
    if (count_vec != count_ref) {
        printf("  ERROR: LT_EXPR unsigned reduction test failed: %u vs %u\n", 
               count_vec, count_ref);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(dest_double, src1_double, src2_double, 7.77, -7.77);
    ref_le_double(ref_double, src1_double, src2_double, 7.77, -7.77);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR double test failed\n");
        errors++;
    }
    
    /* Additional tests for coverage */
    printf("Testing additional comparison patterns...\n");
    
    /* GT with unsigned */
    test_gt_unsigned(dest_uint, src1_uint, src2_uint);
    for (int i = 0; i < N; i++) {
        uint32_t expected = src1_uint[i] > src2_uint[i] ? 0xFFFFFFFF : 0;
        if (dest_uint[i] != expected) {
            printf("  ERROR: GT_EXPR unsigned test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* LE with signed integers */
    test_le_int(dest_int, src1_int, src2_int);
    for (int i = 0; i < N; i++) {
        int expected = src1_int[i] <= src2_int[i] ? -1 : 0;
        if (dest_int[i] != expected) {
            printf("  ERROR: LE_EXPR signed integer test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* GE with doubles */
    test_ge_double(dest_double, src1_double, src2_double);
    for (int i = 0; i < N; i++) {
        double expected = src1_double[i] >= src2_double[i] ? 2.0 : -2.0;
        if (dest_double[i] != expected) {
            printf("  ERROR: GE_EXPR double test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* LT with floats */
    test_lt_float(dest_float, src1_float, src2_float);
    for (int i = 0; i < N; i++) {
        float expected = src1_float[i] < src2_float[i] ? 3.14f : -3.14f;
        if (dest_float[i] != expected) {
            printf("  ERROR: LT_EXPR float test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* Prevent dead code elimination */
    escape(dest_int);
    escape(dest_uint);
    escape(dest_float);
    escape(dest_double);
    escape(&count_vec);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        printf("The vectorizer should have transformed GT, GE, LT, and LE comparisons\n");
        printf("into bitwise operations as per the uncovered code in tree-vect-stmts.cc\n");
    } else {
        printf("\n%d test(s) failed\n", errors);
    }
    
    return errors;
}
