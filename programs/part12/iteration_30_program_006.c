/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent dead code elimination */
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
        mask[i] = src1[i] >= src2[i];
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
void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 2: GE_EXPR with floats */
void test_ge_float(int ALIGNED *mask, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] >= src2[i];
    }
    escape(mask);
}

/* Case 3: LT_EXPR with unsigned integers (reduction pattern) */
unsigned test_lt_unsigned(const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    unsigned count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles */
void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2, double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */

/* Case 5: LT_EXPR with signed integers (should trigger swap) */
void test_lt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 6: LE_EXPR with signed integers (should trigger swap) */
void test_le_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 7: Mixed pattern with GE_EXPR and unsigned */
void test_ge_unsigned(unsigned ALIGNED *dest, const unsigned ALIGNED *src1, const unsigned ALIGNED *src2, unsigned val1, unsigned val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 8: GT_EXPR with floats */
void test_gt_float(float ALIGNED *dest, const float ALIGNED *src1, const float ALIGNED *src2, float val1, float val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
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
    unsigned *ref_uint = aligned_alloc(32, N * sizeof(unsigned));
    
    float *src1_float = aligned_alloc(32, N * sizeof(float));
    float *src2_float = aligned_alloc(32, N * sizeof(float));
    float *dest_float = aligned_alloc(32, N * sizeof(float));
    float *ref_float = aligned_alloc(32, N * sizeof(float));
    
    double *src1_double = aligned_alloc(32, N * sizeof(double));
    double *src2_double = aligned_alloc(32, N * sizeof(double));
    double *dest_double = aligned_alloc(32, N * sizeof(double));
    double *ref_double = aligned_alloc(32, N * sizeof(double));
    
    /* Initialize with patterned data to create mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer patterns */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i + 1 : (i % 3 == 1) ? i - 1 : i;
        
        /* Unsigned patterns */
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 : (i % 4 == 1) ? i * 2 - 1 : i * 2;
        
        /* Float patterns */
        src1_float[i] = i * 0.5f;
        src2_float[i] = (i % 5 == 0) ? i * 0.5f + 0.1f : (i % 5 == 1) ? i * 0.5f - 0.1f : i * 0.5f;
        
        /* Double patterns */
        src1_double[i] = i * 0.25;
        src2_double[i] = (i % 7 == 0) ? i * 0.25 + 0.05 : (i % 7 == 1) ? i * 0.25 - 0.05 : i * 0.25;
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
    printf("Test 3: LT_EXPR with unsigned (reduction)... ");
    unsigned vec_count = test_lt_unsigned(src1_uint, src2_uint);
    unsigned ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("FAILED (vec=%u, ref=%u)\n", vec_count, ref_count);
        errors++;
    } else {
        printf("PASSED (%u)\n", vec_count);
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Test 4: LE_EXPR with doubles... ");
    test_le_double(dest_double, src1_double, src2_double, 1.0, 2.0);
    ref_le_double(ref_double, src1_double, src2_double, 1.0, 2.0);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 5: LT_EXPR with signed integers (should trigger swap) */
    printf("Test 5: LT_EXPR with signed integers... ");
    test_lt_int(dest_int, src1_int, src2_int, 300, 400);
    /* Reuse ref_gt_int with swapped arguments for LT check */
    for (i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] < src2_int[i] ? 300 : 400;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 6: LE_EXPR with signed integers (should trigger swap) */
    printf("Test 6: LE_EXPR with signed integers... ");
    test_le_int(dest_int, src1_int, src2_int, 500, 600);
    for (i = 0; i < N; i++) {
        ref_int[i] = src1_int[i] <= src2_int[i] ? 500 : 600;
    }
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 7: GE_EXPR with unsigned */
    printf("Test 7: GE_EXPR with unsigned... ");
    test_ge_unsigned(dest_uint, src1_uint, src2_uint, 700, 800);
    for (i = 0; i < N; i++) {
        ref_uint[i] = src1_uint[i] >= src2_uint[i] ? 700 : 800;
    }
    if (memcmp(dest_uint, ref_uint, N * sizeof(unsigned)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 8: GT_EXPR with floats */
    printf("Test 8: GT_EXPR with floats... ");
    test_gt_float(dest_float, src1_float, src2_float, 9.0f, 10.0f);
    for (i = 0; i < N; i++) {
        ref_float[i] = src1_float[i] > src2_float[i] ? 9.0f : 10.0f;
    }
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("FAILED\n");
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
    free(ref_uint);
    free(src1_float);
    free(src2_float);
    free(dest_float);
    free(ref_float);
    free(src1_double);
    free(src2_double);
    free(dest_double);
    free(ref_double);
    
    printf("\nTotal errors: %d\n", errors);
    return errors;
}
