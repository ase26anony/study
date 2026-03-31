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

static void ref_ge_float(int *mask, const float *src1, const float *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
}

static unsigned ref_lt_unsigned(const unsigned *src1, const unsigned *src2) {
    unsigned count = 0;
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

/* Test kernels targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    int val1 = 0xABCDEF;
    int val2 = 0x123456;
    
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats - should trigger bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_ge_float(int ALIGNED *mask, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Case 3: LT_EXPR with unsigned integers - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_AND_EXPR */
static unsigned test_lt_unsigned(const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]) ? 1 : 0;
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles - should trigger swap and bitop1 = BIT_NOT_EXPR, bitop2 = BIT_IOR_EXPR */
static void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2) {
    double val1 = 3.141592653589793;
    double val2 = 2.718281828459045;
    
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for mixed patterns */

/* Mixed comparisons in same loop to test multiple transformations */
static void test_mixed_comparisons(int ALIGNED *results, 
                                  const int ALIGNED *a, 
                                  const int ALIGNED *b,
                                  const float ALIGNED *fa,
                                  const float ALIGNED *fb) {
    for (int i = 0; i < N; i++) {
        int r = 0;
        r |= (a[i] > b[i]) ? 0x1 : 0;
        r |= (a[i] >= b[i]) ? 0x2 : 0;
        r |= (a[i] < b[i]) ? 0x4 : 0;
        r |= (a[i] <= b[i]) ? 0x8 : 0;
        r |= (fa[i] > fb[i]) ? 0x10 : 0;
        r |= (fa[i] >= fb[i]) ? 0x20 : 0;
        r |= (fa[i] < fb[i]) ? 0x40 : 0;
        r |= (fa[i] <= fb[i]) ? 0x80 : 0;
        results[i] = r;
    }
    escape(results);
}

/* Test with different data patterns to ensure both true and false comparisons */
static void initialize_data(int ALIGNED *int1, int ALIGNED *int2,
                           unsigned ALIGNED *uint1, unsigned ALIGNED *uint2,
                           float ALIGNED *float1, float ALIGNED *float2,
                           double ALIGNED *double1, double ALIGNED *double2) {
    for (int i = 0; i < N; i++) {
        /* Create alternating patterns to get mixed comparison results */
        int1[i] = (i % 3 == 0) ? i + 10 : i - 5;
        int2[i] = i;
        
        uint1[i] = (i % 4 == 0) ? i + 20 : i;
        uint2[i] = (i % 5 == 0) ? i + 10 : i + 5;
        
        float1[i] = (i % 2 == 0) ? i * 1.5f : i * 0.75f;
        float2[i] = i * 1.0f;
        
        double1[i] = (i % 3 == 0) ? i * 2.0 : i * 0.5;
        double2[i] = i * 1.0;
    }
}

int main() {
    /* Allocate aligned arrays */
    int ALIGNED *int_src1 = (int*)aligned_alloc(32, N * sizeof(int));
    int ALIGNED *int_src2 = (int*)aligned_alloc(32, N * sizeof(int));
    unsigned ALIGNED *uint_src1 = (unsigned*)aligned_alloc(32, N * sizeof(unsigned));
    unsigned ALIGNED *uint_src2 = (unsigned*)aligned_alloc(32, N * sizeof(unsigned));
    float ALIGNED *float_src1 = (float*)aligned_alloc(32, N * sizeof(float));
    float ALIGNED *float_src2 = (float*)aligned_alloc(32, N * sizeof(float));
    double ALIGNED *double_src1 = (double*)aligned_alloc(32, N * sizeof(double));
    double ALIGNED *double_src2 = (double*)aligned_alloc(32, N * sizeof(double));
    
    /* Results arrays */
    int ALIGNED *int_results = (int*)aligned_alloc(32, N * sizeof(int));
    int ALIGNED *int_results_ref = (int*)aligned_alloc(32, N * sizeof(int));
    int ALIGNED *float_mask = (int*)aligned_alloc(32, N * sizeof(int));
    int ALIGNED *float_mask_ref = (int*)aligned_alloc(32, N * sizeof(int));
    double ALIGNED *double_results = (double*)aligned_alloc(32, N * sizeof(double));
    double ALIGNED *double_results_ref = (double*)aligned_alloc(32, N * sizeof(double));
    int ALIGNED *mixed_results = (int*)aligned_alloc(32, N * sizeof(int));
    
    if (!int_src1 || !int_src2 || !uint_src1 || !uint_src2 ||
        !float_src1 || !float_src2 || !double_src1 || !double_src2 ||
        !int_results || !int_results_ref || !float_mask || !float_mask_ref ||
        !double_results || !double_results_ref || !mixed_results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize test data */
    initialize_data(int_src1, int_src2, uint_src1, uint_src2,
                   float_src1, float_src2, double_src1, double_src2);
    
    int errors = 0;
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(int_results, int_src1, int_src2);
    ref_gt_int(int_results_ref, int_src1, int_src2, 0xABCDEF, 0x123456);
    if (memcmp(int_results, int_results_ref, N * sizeof(int)) != 0) {
        fprintf(stderr, "GT_EXPR test failed!\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(float_mask, float_src1, float_src2);
    ref_ge_float(float_mask_ref, float_src1, float_src2);
    if (memcmp(float_mask, float_mask_ref, N * sizeof(int)) != 0) {
        fprintf(stderr, "GE_EXPR test failed!\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers */
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count = test_lt_unsigned(uint_src1, uint_src2);
    unsigned count_ref = ref_lt_unsigned(uint_src1, uint_src2);
    if (count != count_ref) {
        fprintf(stderr, "LT_EXPR test failed: %u != %u\n", count, count_ref);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(double_results, double_src1, double_src2);
    ref_le_double(double_results_ref, double_src1, double_src2, 
                  3.141592653589793, 2.718281828459045);
    if (memcmp(double_results, double_results_ref, N * sizeof(double)) != 0) {
        fprintf(stderr, "LE_EXPR test failed!\n");
        errors++;
    }
    
    /* Test 5: Mixed comparisons */
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(mixed_results, int_src1, int_src2, float_src1, float_src2);
    escape(mixed_results);  /* Prevent optimization */
    
    /* Clean up */
    free(int_src1);
    free(int_src2);
    free(uint_src1);
    free(uint_src2);
    free(float_src1);
    free(float_src2);
    free(double_src1);
    free(double_src2);
    free(int_results);
    free(int_results_ref);
    free(float_mask);
    free(float_mask_ref);
    free(double_results);
    free(double_results_ref);
    free(mixed_results);
    
    if (errors == 0) {
        printf("All tests passed!\n");
        return 0;
    } else {
        fprintf(stderr, "%d test(s) failed\n", errors);
        return 1;
    }
}
