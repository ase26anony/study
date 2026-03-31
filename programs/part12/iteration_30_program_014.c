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

static void ref_le_double(double *dest, const double *src1, const double *src2, 
                          double true_val, double false_val) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? true_val : false_val;
    }
}

/* Test kernels targeting specific comparison transformations */

/* GT_EXPR transformation for integers */
static void test_gt_int(int ALIGNED *dest, const int ALIGNED *src1, 
                        const int ALIGNED *src2, int val1, int val2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] > src2[i] ? val1 : val2;
    }
    escape(dest);
}

/* GE_EXPR transformation for floats */
static void test_ge_float(float ALIGNED *dest, const float ALIGNED *src1, 
                          const float ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] >= src2[i] ? 1.0f : 0.0f;
    }
    escape(dest);
}

/* LT_EXPR transformation for unsigned integers (with potential swap) */
static int test_lt_unsigned(const unsigned int ALIGNED *src1, 
                            const unsigned int ALIGNED *src2) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        count += (src1[i] < src2[i]);
    }
    escape(&count);
    return count;
}

/* LE_EXPR transformation for doubles (with potential swap) */
static void test_le_double(double ALIGNED *dest, const double ALIGNED *src1, 
                           const double ALIGNED *src2, double true_val, 
                           double false_val) {
    for (int i = 0; i < N; i++) {
        dest[i] = src1[i] <= src2[i] ? true_val : false_val;
    }
    escape(dest);
}

/* Additional test: Mixed comparisons in same loop */
static void test_mixed_comparisons(int ALIGNED *dest, const int ALIGNED *src1, 
                                   const int ALIGNED *src2) {
    for (int i = 0; i < N; i++) {
        /* This should trigger multiple comparison transformations */
        dest[i] = (src1[i] > src2[i]) ? 1 : 
                  (src1[i] >= src2[i]) ? 2 :
                  (src1[i] < src2[i]) ? 3 :
                  (src1[i] <= src2[i]) ? 4 : 0;
    }
    escape(dest);
}

/* Initialize arrays with pattern that creates mixed comparison results */
static void init_arrays(int ALIGNED *src1_int, int ALIGNED *src2_int,
                        unsigned int ALIGNED *src1_uint, unsigned int ALIGNED *src2_uint,
                        float ALIGNED *src1_float, float ALIGNED *src2_float,
                        double ALIGNED *src1_double, double ALIGNED *src2_double) {
    for (int i = 0; i < N; i++) {
        /* Pattern: alternating greater/less/equal relationships */
        src1_int[i] = i;
        src2_int[i] = (i % 3 == 0) ? i - 1 :  /* src1 > src2 */
                      (i % 3 == 1) ? i + 1 :  /* src1 < src2 */
                      i;                      /* src1 == src2 */
        
        src1_uint[i] = i * 2;
        src2_uint[i] = (i % 4 == 0) ? i * 2 + 1 :
                       (i % 4 == 1) ? i * 2 - 1 :
                       (i % 4 == 2) ? i * 2 + 5 :
                       i * 2;
        
        src1_float[i] = (float)i * 1.5f;
        src2_float[i] = (float)i * ((i % 5 == 0) ? 2.0f :
                                     (i % 5 == 1) ? 1.0f :
                                     (i % 5 == 2) ? 1.5f :
                                     (i % 5 == 3) ? 1.8f : 1.2f);
        
        src1_double[i] = (double)i * 0.75;
        src2_double[i] = (double)i * ((i % 6 == 0) ? 0.5 :
                                      (i % 6 == 1) ? 1.0 :
                                      (i % 6 == 2) ? 0.75 :
                                      (i % 6 == 3) ? 0.8 :
                                      (i % 6 == 4) ? 0.7 : 0.9);
    }
}

int main() {
    /* Aligned arrays for vector loads */
    int ALIGNED src1_int[N], src2_int[N], dest_int[N], ref_int[N];
    unsigned int ALIGNED src1_uint[N], src2_uint[N];
    float ALIGNED src1_float[N], src2_float[N], dest_float[N], ref_float[N];
    double ALIGNED src1_double[N], src2_double[N], dest_double[N], ref_double[N];
    int ALIGNED dest_mixed[N];
    
    int errors = 0;
    
    /* Initialize with mixed comparison patterns */
    init_arrays(src1_int, src2_int, src1_uint, src2_uint,
                src1_float, src2_float, src1_double, src2_double);
    
    /* Test 1: GT_EXPR with integers */
    printf("Testing GT_EXPR (int)...\n");
    test_gt_int(dest_int, src1_int, src2_int, 99, -99);
    ref_gt_int(ref_int, src1_int, src2_int, 99, -99);
    if (memcmp(dest_int, ref_int, N * sizeof(int)) != 0) {
        printf("  ERROR: GT_EXPR test failed\n");
        errors++;
    }
    
    /* Test 2: GE_EXPR with floats */
    printf("Testing GE_EXPR (float)...\n");
    test_ge_float(dest_float, src1_float, src2_float);
    ref_ge_float(ref_float, src1_float, src2_float);
    if (memcmp(dest_float, ref_float, N * sizeof(float)) != 0) {
        printf("  ERROR: GE_EXPR test failed\n");
        errors++;
    }
    
    /* Test 3: LT_EXPR with unsigned integers (reduction pattern) */
    printf("Testing LT_EXPR (unsigned int)...\n");
    int vec_count = test_lt_unsigned(src1_uint, src2_uint);
    int ref_count = ref_lt_unsigned(src1_uint, src2_uint);
    if (vec_count != ref_count) {
        printf("  ERROR: LT_EXPR test failed: vec=%d, ref=%d\n", vec_count, ref_count);
        errors++;
    }
    
    /* Test 4: LE_EXPR with doubles */
    printf("Testing LE_EXPR (double)...\n");
    test_le_double(dest_double, src1_double, src2_double, 3.14159, -2.71828);
    ref_le_double(ref_double, src1_double, src2_double, 3.14159, -2.71828);
    if (memcmp(dest_double, ref_double, N * sizeof(double)) != 0) {
        printf("  ERROR: LE_EXPR test failed\n");
        errors++;
    }
    
    /* Test 5: Mixed comparisons in one loop */
    printf("Testing mixed comparisons...\n");
    test_mixed_comparisons(dest_mixed, src1_int, src2_int);
    
    /* Verify a few sample points */
    for (int i = 0; i < 10; i++) {
        int expected = (src1_int[i] > src2_int[i]) ? 1 :
                      (src1_int[i] >= src2_int[i]) ? 2 :
                      (src1_int[i] < src2_int[i]) ? 3 :
                      (src1_int[i] <= src2_int[i]) ? 4 : 0;
        if (dest_mixed[i] != expected) {
            printf("  ERROR: Mixed comparison mismatch at i=%d: got=%d, expected=%d\n",
                   i, dest_mixed[i], expected);
            errors++;
            break;
        }
    }
    
    /* Additional test: GT with floating point */
    printf("Testing GT_EXPR (float)...\n");
    for (int i = 0; i < N; i++) {
        dest_float[i] = src1_float[i] > src2_float[i] ? 10.0f : -10.0f;
    }
    escape(dest_float);
    
    /* Additional test: LE with integers */
    printf("Testing LE_EXPR (int)...\n");
    for (int i = 0; i < N; i++) {
        dest_int[i] = src1_int[i] <= src2_int[i] ? 777 : 333;
    }
    escape(dest_int);
    
    if (errors == 0) {
        printf("\nAll tests passed successfully!\n");
        printf("The vector comparison transformations (GT, GE, LT, LE) should have been triggered.\n");
    } else {
        printf("\n%d test(s) failed!\n", errors);
    }
    
    return errors;
}
