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
        count += src1[i] < src2[i];
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
static void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    const int val1 = 0xABCD;
    const int val2 = 0x1234;
    
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
        count += src1[i] < src2[i];
    }
    escape(&count);
    return count;
}

/* Case 4: LE_EXPR with doubles */
static void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, const double ALIGNED *src2) {
    const double val1 = 3.14159;
    const double val2 = 2.71828;
    
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Additional tests for signed integer comparisons */

/* Case 5: LT_EXPR with signed integers (should trigger std::swap) */
static void test_lt_int(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2) {
    const int val1 = 0xDEAD;
    const int val2 = 0xBEEF;
    
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] < src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* Case 6: LE_EXPR with signed integers (should trigger std::swap) */
static void test_le_int(int ALIGNED *mask, const int ALIGNED *src1, const int ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        mask[i] = src1[i] <= src2[i] ? -1 : 0;
    }
    escape(mask);
}

/* Case 7: Mixed pattern with GT and GE in same loop */
static void test_mixed_comparisons(int ALIGNED *dest, const int ALIGNED *src1, const int ALIGNED *src2, 
                                   const float ALIGNED *fsrc1, const float ALIGNED *fsrc2) {
    for (int i = 0; i < N; i++) {
        /* Mix GT and GE in same loop to test multiple transformations */
        int cond1 = src1[i] > src2[i] ? 1 : 0;
        int cond2 = fsrc1[i] >= fsrc2[i] ? 2 : 0;
        dest[i] = cond1 + cond2;
    }
    escape(dest);
}

/* Initialize arrays with pattern that creates mixed comparison results */
static void init_arrays(int ALIGNED *int1, int ALIGNED *int2,
                        unsigned ALIGNED *uint1, unsigned ALIGNED *uint2,
                        float ALIGNED *float1, float ALIGNED *float2,
                        double ALIGNED *double1, double ALIGNED *double2) {
    for (int i = 0; i < N; i++) {
        /* Create alternating patterns to ensure both true and false comparisons */
        int1[i] = (i % 3 == 0) ? i + 10 : i - 5;
        int2[i] = i;
        
        uint1[i] = (i % 4 == 0) ? i + 20 : i;
        uint2[i] = (i % 5 == 0) ? i - 10 : i + 5;
        
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
    
    /* Result arrays */
    int ALIGNED *int_dest = (int*)aligned_alloc(32, N * sizeof(int));
    int ALIGNED *int_dest_ref = (int*)aligned_alloc(32, N * sizeof(int));
    int ALIGNED *int_mask = (int*)aligned_alloc(32, N * sizeof(int));
    int ALIGNED *int_mask_ref = (int*)aligned_alloc(32, N * sizeof(int));
    double ALIGNED *double_dest = (double*)aligned_alloc(32, N * sizeof(double));
    double ALIGNED *double_dest_ref = (double*)aligned_alloc(32, N * sizeof(double));
    
    /* Initialize source arrays */
    init_arrays(int_src1, int_src2, uint_src1, uint_src2, 
                float_src1, float_src2, double_src1, double_src2);
    
    int errors = 0;
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR with integers...\n");
    test_gt_int(int_dest, int_src1, int_src2);
    ref_gt_int(int_dest_ref, int_src1, int_src2, 0xABCD, 0x1234);
    if (memcmp(int_dest, int_dest_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR test failed!\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR with floats...\n");
    test_ge_float(int_mask, float_src1, float_src2);
    ref_ge_float(int_mask_ref, float_src1, float_src2);
    if (memcmp(int_mask, int_mask_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: GE_EXPR test failed!\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction) */
    printf("Testing LT_EXPR with unsigned integers...\n");
    unsigned count = test_lt_unsigned(uint_src1, uint_src2);
    unsigned ref_count = ref_lt_unsigned(uint_src1, uint_src2);
    if (count != ref_count) {
        printf("  ERROR: LT_EXPR reduction test failed! Got %u, expected %u\n", count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR with doubles...\n");
    test_le_double(double_dest, double_src1, double_src2);
    ref_le_double(double_dest_ref, double_src1, double_src2, 3.14159, 2.71828);
    if (memcmp(double_dest, double_dest_ref, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR test failed!\n");
        errors++;
    }
    
    /* Test 5: LT_EXPR with signed integers */
    printf("Testing LT_EXPR with signed integers...\n");
    test_lt_int(int_dest, int_src1, int_src2);
    ref_gt_int(int_dest_ref, int_src2, int_src1, 0xDEAD, 0xBEEF);  /* Note: swapped for LT */
    if (memcmp(int_dest, int_dest_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: LT_EXPR signed test failed!\n");
        errors++;
    }
    
    /* Test 6: LE_EXPR with signed integers */
    printf("Testing LE_EXPR with signed integers...\n");
    test_le_int(int_mask, int_src1, int_src2);
    ref_ge_float(int_mask_ref, (float*)int_src2, (float*)int_src1);  /* Note: swapped and float cast */
    /* This comparison is less precise due to float casting, so we do element-wise */
    for (int i = 0; i < N; i++) {
        int expected = int_src1[i] <= int_src2[i] ? -1 : 0;
        if (int_mask[i] != expected) {
            printf("  ERROR: LE_EXPR signed test failed at index %d\n", i);
            errors++;
            break;
        }
    }
    
    /* Test 7: Mixed comparisons */
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(int_dest, int_src1, int_src2, float_src1, float_src2);
    /* Reference computation */
    for (int i = 0; i < N; i++) {
        int cond1 = int_src1[i] > int_src2[i] ? 1 : 0;
        int cond2 = float_src1[i] >= float_src2[i] ? 2 : 0;
        int_dest_ref[i] = cond1 + cond2;
    }
    if (memcmp(int_dest, int_dest_ref, N * sizeof(int)) != 0) {
        printf("  ERROR: Mixed comparisons test failed!\n");
        errors++;
    }
    
    /* Cleanup */
    free(int_src1);
    free(int_src2);
    free(uint_src1);
    free(uint_src2);
    free(float_src1);
    free(float_src2);
    free(double_src1);
    free(double_src2);
    free(int_dest);
    free(int_dest_ref);
    free(int_mask);
    free(int_mask_ref);
    free(double_dest);
    free(double_dest_ref);
    
    if (errors == 0) {
        printf("\nAll tests passed!\n");
        return 0;
    } else {
        printf("\n%d test(s) failed!\n", errors);
        return 1;
    }
}
