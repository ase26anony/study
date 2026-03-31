/* Test program to trigger vector comparison transformations for GT, GE, LT, LE operations */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent compiler from optimizing away computations */
static void escape(void *p) {
    __asm__ volatile ("" : : "r"(p) : "memory");
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
void test_ge_float(float ALIGNED *dest, const float ALIGNED *src1, const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
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
void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2, 
                    double val1, double val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests to ensure all paths are covered */

/* GT_EXPR with mixed results */
void test_gt_mixed(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? i : -i;
    }
    escape(dest);
}

/* GE_EXPR with unsigned integers */
void test_ge_unsigned(unsigned ALIGNED *dest, const unsigned ALIGNED *src1, const unsigned ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 0xFFFFFFFF : 0;
    }
    escape(dest);
}

/* LT_EXPR with floats (conditional assignment) */
void test_lt_float(float ALIGNED *dest, const float ALIGNED *src1, const float ALIGNED *src2,
                   float threshold) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? src1[i] : threshold;
    }
    escape(dest);
}

/* LE_EXPR with signed integers (mask generation) */
void test_le_int_mask(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i];
    }
    escape(dest);
}

/* Initialize arrays with patterned data to create mixed comparison results */
static void init_arrays() {
    /* Arrays will be initialized in main to avoid static initialization overhead */
}

int main() {
    int i;
    int errors = 0;
    
    /* Allocate aligned arrays */
    int *src1_int = aligned_alloc(32, N * sizeof(int));
    int *src2_int = aligned_alloc(32, N * sizeof(int));
    int *dest_int = aligned_alloc(32, N * sizeof(int));
    int *ref_int = aligned_alloc(32, N * sizeof(int));
    
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
    
    /* Initialize with patterned data to ensure mixed comparison results */
    for (i = 0; i < N; i++) {
        /* Integer patterns: alternating greater/lesser relationships */
        src1_int[i] = (i % 4 == 0) ? i + 10 : 
                     (i % 4 == 1) ? i - 5 :
                     (i % 4 == 2) ? i : i + 100;
        src2_int[i] = i;
        
        /* Unsigned patterns */
        src1_uint[i] = (i % 3 == 0) ? i * 2 : 
                      (i % 3 == 1) ? i / 2 : i;
        src2_uint[i] = i + 1;
        
        /* Float patterns with NaN avoidance */
        src1_float[i] = (i % 5 == 0) ? i * 1.5f :
                       (i % 5 == 1) ? i * 0.5f :
                       (i % 5 == 2) ? i * 2.0f :
                       (i % 5 == 3) ? i * 0.8f : i * 1.2f;
        src2_float[i] = i * 1.0f;
        
        /* Double patterns */
        src1_double[i] = (i % 6 == 0) ? i * 3.14 :
                        (i % 6 == 1) ? i * 0.5 :
                        (i % 6 == 2) ? i * 2.71 :
                        (i % 6 == 3) ? i * 1.41 :
                        (i % 6 == 4) ? i * 0.99 : i * 1.73;
        src2_double[i] = i * 1.0;
    }
    
    printf("Testing vector comparison transformations...\n");
    
    /* Test 1: GT_EXPR with integers */
    printf("Test 1: GT_EXPR with integers... ");
    test_gt_int(dest_int, src1_int, src2_int, 100, -100);
    ref_gt_int(ref_int, src1_int, src2_int, 100, -100);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Test 2: GE_EXPR with floats... ");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
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
    
    /* Additional tests for comprehensive coverage */
    
    /* Test 5: GT_EXPR with mixed results */
    printf("Test 5: GT_EXPR with mixed assignment... ");
    test_gt_mixed(dest_int, src1_int, src2_int);
    /* No reference check for this one - just ensure it compiles and runs */
    printf("EXECUTED\n");
    
    /* Test 6: GE_EXPR with unsigned integers */
    printf("Test 6: GE_EXPR with unsigned integers... ");
    test_ge_unsigned(dest_uint, src1_uint, src2_uint);
    /* Simple check that something was written */
    int sum = 0;
    for (i = 0; i < N; i++) sum += dest_uint[i];
    if (sum == 0) {
        printf("SUSPICIOUS (all zeros)\n");
    } else {
        printf("EXECUTED\n");
    }
    
    /* Test 7: LT_EXPR with floats */
    printf("Test 7: LT_EXPR with floats... ");
    test_lt_float(dest_float, src1_float, src2_float, 42.0f);
    /* Check a few values */
    int ok = 1;
    for (i = 0; i < 10 && ok; i++) {
        float expected = src1_float[i] < src2_float[i] ? src1_float[i] : 42.0f;
        if (dest_float[i] != expected) ok = 0;
    }
    if (!ok) {
        printf("FAILED\n");
        errors++;
    } else {
        printf("PASSED\n");
    }
    
    /* Test 8: LE_EXPR with integer mask */
    printf("Test 8: LE_EXPR with integer mask... ");
    test_le_int_mask(dest_int, src1_int, src2_int);
    /* Check a few values */
    ok = 1;
    for (i = 0; i < 10 && ok; i++) {
        int expected = src1_int[i] <= src2_int[i];
        if (dest_int[i] != expected) ok = 0;
    }
    if (!ok) {
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
    return errors > 0 ? 1 : 0;
}
