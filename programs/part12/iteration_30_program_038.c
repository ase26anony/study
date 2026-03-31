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

static void ref_le_double(double *dest, const double *src1, const double *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
}

/* Vectorizable test functions targeting specific uncovered cases */

/* Case 1: GT_EXPR with integers */
static void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats */
static void test_ge_float(int ALIGNED *mask, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Case 3: LT_EXPR with unsigned integers (reduction pattern) */
static unsigned test_lt_unsigned(const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles */
static void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional test cases to ensure all paths are covered */

/* GT_EXPR with unsigned integers */
static void test_gt_unsigned(int ALIGNED *dest, const unsigned ALIGNED *src1, const unsigned ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR with signed integers (reduction) */
static int test_ge_int_reduction(const int ALIGNED *src1, const int ALIGNED *src2) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (src1[i] >= src2[i]);
    }
    escape(&sum);
    return sum;
}

/* LT_EXPR with floats (mask generation) */
static void test_lt_float_mask(int ALIGNED *mask, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] < src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* LE_EXPR with unsigned (conditional assignment) */
static void test_le_unsigned(unsigned ALIGNED *dest, const unsigned ALIGNED *src1, const unsigned ALIGNED *src2, unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Mixed comparisons in same loop to trigger different transformations */
static void test_mixed_comparisons(int ALIGNED *results, const int ALIGNED *a, const int ALIGNED *b, const float ALIGNED *fa, const float ALIGNED *fb) {
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

int main() {
    int i;
    int errors = 0;
    
    /* Allocate aligned arrays */
    int *src1_int = aligned_alloc(32, N * sizeof(int));
    int *src2_int = aligned_alloc(32, N * sizeof(int));
    int *dest_int = aligned_alloc(32, N * sizeof(int));
    int *ref_int = aligned_alloc(32, N * sizeof(int));
    int *mask_int = aligned_alloc(32, N * sizeof(int));
    int *ref_mask = aligned_alloc(32, N * sizeof(int));
    
    unsigned *src1_uint = aligned_alloc(32, N * sizeof(unsigned));
    unsigned *src2_uint = aligned_alloc(32, N * sizeof(unsigned));
    unsigned *dest_uint = aligned_alloc(32, N * sizeof(unsigned));
    
    float *src1_float = aligned_alloc(32, N * sizeof(float));
    float *src2_float = aligned_alloc(32, N * sizeof(float));
    
    double *src1_double = aligned_alloc(32, N * sizeof(double));
    double *src2_double = aligned_alloc(32, N * sizeof(double));
    double *dest_double = aligned_alloc(32, N * sizeof(double));
    double *ref_double = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (i = 0; i < N; i++) {
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        src1_float[i] = (i % 5 == 0) ? i * 1.5f : (i % 5 == 1) ? i * 0.5f : i * 1.0f;
        src2_float[i] = (i % 5 == 2) ? i * 1.6f : (i % 5 == 3) ? i * 0.4f : i * 1.0f;
        
        src1_double[i] = (i % 7 == 0) ? i * 2.5 : (i % 7 == 1) ? i * 1.5 : i * 2.0;
        src2_double[i] = (i % 7 == 2) ? i * 2.6 : (i % 7 == 3) ? i * 1.4 : i * 2.0;
    }
    
    printf("Testing vector comparison transformations...\n");
    
    /* Test 1: GT_EXPR with integers */
    printf("Test 1: GT_EXPR with integers... ");
    test_gt_int(dest_int, src1_int, src2_int, 100, 200);
    ref_gt_int(ref_int, src1_int, src2_int, 100, 200);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Test 2: GE_EXPR with floats... ");
    test_ge_float(mask_int, src1_float, src2_float);
    ref_ge_float(ref_mask, src1_float, src2_float);
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    printf("Test 3: LT_EXPR with unsigned integers... ");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("FAILED (vec=%u, ref=%u)\n", vec_count, ref_count);
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Test 4: LE_EXPR with doubles... ");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, 2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, 2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 5: GT_EXPR with unsigned integers */
    printf("Test 5: GT_EXPR with unsigned integers... ");
    test_gt_unsigned(dest_int, src1_uint, src2_uint, 300, 400);
    ref_gt_int(ref_int, (int*)src1_uint, (int*)src2_uint, 300, 400);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 6: GE_EXPR with signed integers (reduction) */
    printf("Test 6: GE_EXPR with signed integers (reduction)... ");
    int vec_sum = test_ge_int_reduction(src1_int, src2_int);
    int ref_sum = 0;
    for (i = 0; i < N; i++) {
        ref_sum += (src1_int[i] >= src2_int[i]);
    }
    if (vec_sum != ref_sum) {
        printf("FAILED (vec=%d, ref=%d)\n", vec_sum, ref_sum);
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 7: LT_EXPR with floats (mask generation) */
    printf("Test 7: LT_EXPR with floats... ");
    test_lt_float_mask(mask_int, src1_float, src2_float);
    for (i = 0; i < N; i++) {
        ref_mask[i] = src1_float[i] < src2_float[i] ? -1 : 0;
    }
    if (memcmp(mask_int, ref_mask, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 8: LE_EXPR with unsigned integers */
    printf("Test 8: LE_EXPR with unsigned integers... ");
    test_le_unsigned(dest_uint, src1_uint, src2_uint, 500, 600);
    for (i = 0; i < N; i++) {
        ref_int[i] = src1_uint[i] <= src2_uint[i] ? 500 : 600;
    }
    if (memcmp(dest_uint, ref_int, N * sizeof(unsigned)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 9: Mixed comparisons */
    printf("Test 9: Mixed comparisons in same loop... ");
    test_mixed_comparisons(dest_int, src1_int, src2_int, src1_float, src2_float);
    /* Verify a few samples */
    int sample_verified = 1;
    for (i = 0; i < N; i += N/10) {
        int expected = 0;
        expected |= (src1_int[i] > src2_int[i]) ? 0x1 : 0;
        expected |= (src1_int[i] >= src2_int[i]) ? 0x2 : 0;
        expected |= (src1_int[i] < src2_int[i]) ? 0x4 : 0;
        expected |= (src1_int[i] <= src2_int[i]) ? 0x8 : 0;
        expected |= (src1_float[i] > src2_float[i]) ? 0x10 : 0;
        expected |= (src1_float[i] >= src2_float[i]) ? 0x20 : 0;
        expected |= (src1_float[i] < src2_float[i]) ? 0x40 : 0;
        expected |= (src1_float[i] <= src2_float[i]) ? 0x80 : 0;
        if (dest_int[i] != expected) {
            sample_verified = 0;
            break;
        }
    }
    if (!sample_verified) {
        printf("FAILED at i=%d\n", i);
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Cleanup */
    free(src1_int);
    free(src2_int);
    free(dest_int);
    free(ref_int);
    free(mask_int);
    free(ref_mask);
    free(src1_uint);
    free(src2_uint);
    free(dest_uint);
    free(src1_float);
    free(src2_float);
    free(src1_double);
    free(src2_double);
    free(dest_double);
    free(ref_double);
    
    printf("\nTotal errors: %d\n", errors);
    return errors > 0 ? 1 : 0;
}
